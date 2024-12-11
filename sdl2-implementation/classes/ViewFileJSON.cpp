#include "ViewFileJSON.h"
#include <iostream>
#include <errno.h>
#include "TextNode.h"
#include <fstream>
#include "Util.h"
#include "LabelNode.h"

using namespace sajson;

inline bool success(const document& doc) {
    if (!doc.is_valid()) {
        fprintf(stderr, "%s\n", doc.get_error_message_as_cstring());
        return false;
    }
    return true;
}

ViewFileJSON::ViewFileJSON(std::string filepath, SDL_Renderer* renderer, std::shared_ptr<Font> labelNodeFont)
    : filepath(filepath)
    , renderer(renderer)
    , labelNodeFont(labelNodeFont)
{
}

bool ViewFileJSON::read(std::vector<std::shared_ptr<Node>> &nodes, std::vector<std::shared_ptr<Arrow>>& arrows) {
    FILE* file = fopen(filepath.c_str(), "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file\n");
        perror("error message");
        return false;
    }
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = new char[length];
    if (length != fread(buffer, 1, length, file)) {
        fclose(file);
        delete[] buffer;
        fprintf(stderr, "Failed to read entire file\n");
        return false;
    }
    fclose(file);

    const sajson::document& document = sajson::parse(
        sajson::dynamic_allocation(), mutable_string_view(length, buffer));
    if (!success(document)) {
        fclose(file);
        delete[] buffer;
        return false;
    }

    // Extract nodes
    traverse(nodes, arrows, document.get_root());

    delete[] buffer;

    return true;
}

bool ViewFileJSON::write(std::vector<std::shared_ptr<Node>> &nodes, std::vector<std::shared_ptr<Arrow>>& arrows) {
    std::ofstream file;
    file.open(filepath);
    file << "{\"version\": 0, \"nodes\": [";
    for(size_t i = 0; i < nodes.size(); i++) {
        if(i != 0) file << ", ";
        file << nodes[i]->toString();
    }
    file << "], \"arrows\": [";
    for(size_t i = 0; i < arrows.size(); i++) {
        if(i != 0) file << ", ";
        auto label = arrows[i]->getLabel();
        Util::replace_all(label, "\n", "\\n");
        Util::replace_all(label, "\"", "\\\"");
        file << "{\"label\": \"" << label << "\", ";
        int sourceIndex = -1;
        int targetIndex = -1;
        for(size_t j = 0; j < nodes.size(); j++) {
            if(nodes[j] == arrows[i]->getSourceNode())
                sourceIndex = j;
            if(nodes[j] == arrows[i]->getTargetNode())
                targetIndex = j;
        }
        file << "\"source\": " << sourceIndex << ", ";
        file << "\"target\": " << targetIndex << ", ";
        SDL_FPoint controlPoint = arrows[i]->arrowCurve->getControlPoint();
        file << "\"controlX\": " << controlPoint.x << ", ";
        file << "\"controlY\": " << controlPoint.y << "}";
    }
    file << "]}";
    file.close();
    SDL_Log("Saved successfully");
    return true;
}
    
bool ViewFileJSON::traverse(std::vector<std::shared_ptr<Node>> &nodes, std::vector<std::shared_ptr<Arrow>>& arrows, const sajson::value& view) {
    // Make sure the file has an object
    if(view.get_type() != TYPE_OBJECT) {
        SDL_Log("Error: root JSON object was not object type");
        return false;
    }
    // Iterate through properties
    auto length = view.get_length();
    bool nodeListRead = false;
    bool arrowListRead = false;
    for(auto i = 0u; i < length; i++) {
        auto key = view.get_object_key(i).as_string();
        if(key == "nodes") {
            if(!readNodeList(nodes, view.get_object_value(i))) {
                SDL_Log("Error: unable to read node list");
                return false;
            }
            nodeListRead = true;
        }
        if(key == "arrows") {
            if(!readArrowList(arrows, nodes, view.get_object_value(i))) {
                SDL_Log("Error: unable to read arrow list");
                return false;
            }
            arrowListRead = true;
        }
    }
    if(!nodeListRead) {
        SDL_Log("Error: node list not found in file");
        return false;
    }
    if(!arrowListRead) {
        SDL_Log("Error: arrow list not found in file");
        return false;
    }
    return true;
}

bool ViewFileJSON::readNodeList(std::vector<std::shared_ptr<Node>>& nodes, const sajson::value& nodeList) {
    // Make sure it's a list
    if(nodeList.get_type() != TYPE_ARRAY) {
        SDL_Log("Error: node list in JSON was not array type");
        return false;
    }
    // Iterate through items
    auto length = nodeList.get_length();
    for(size_t i = 0; i < length; i++) {
        if(!readNode(nodes, nodeList.get_array_element(i))) {
            SDL_Log("Error: unable to read node");
            nodes.clear();
            return false;
        }
    }
    return true;
}

bool ViewFileJSON::readNode(std::vector<std::shared_ptr<Node>>& nodes, const sajson::value& node) {
    // Make sure it's an object
    if(node.get_type() != TYPE_OBJECT) {
        SDL_Log("Error: node in JSON was not object type");
        return false;
    }
    // Checklist of things we need
    bool foundX = false;
    int x;
    bool foundY = false;
    int y;
    bool foundText = false;
    bool foundLabel = false;
    std::string content;
    auto length = node.get_length();
    for(auto i = 0u; i < length; i++) {
        auto key = node.get_object_key(i).as_string();
        auto val = node.get_object_value(i);
        if(key == "x") {
            if(val.get_type() == TYPE_INTEGER) {
                x = val.get_number_value();
                foundX = true;
            }
        }
        else if(key == "y") {
            if(val.get_type() == TYPE_INTEGER) {
                y = val.get_number_value();
                foundY = true;
            }
        }
        else if(key == "text") {
            if(val.get_type() == TYPE_STRING) {
                content = val.as_string();
                foundText = true;
            }
        }
        else if(key == "label") {
            if(val.get_type() == TYPE_STRING) {
                content = val.as_string();
                foundLabel = true;
            }
        }
    }
    // Put together node
    if(foundX && foundY) {
        if(foundText) {
            // Make text node
            SDL_Point location;
            location.x = x;
            location.y = y;
            auto newNode = std::shared_ptr<TextNode>(new TextNode(renderer, content, location));
            nodes.push_back(newNode);
            return true;
        }
        else if(foundLabel) {
            // Make label node
            SDL_Point location;
            location.x = x;
            location.y = y;
            auto newNode = std::shared_ptr<LabelNode>(new LabelNode(renderer, labelNodeFont, location, content));
            nodes.push_back(newNode);
            return true;
        }
    }
    return false;
}

bool ViewFileJSON::readArrowList(std::vector<std::shared_ptr<Arrow>>& arrows, std::vector<std::shared_ptr<Node>>& nodes, const sajson::value& arrowList) {
    // Make sure it's a list
    if(arrowList.get_type() != TYPE_ARRAY) {
        SDL_Log("Error: arrow list in JSON was not array type");
        return false;
    }
    // Iterate through items
    auto length = arrowList.get_length();
    for(size_t i = 0; i < length; i++) {
        if(!readArrow(arrows, nodes, arrowList.get_array_element(i))) {
            SDL_Log("Error: unable to read arrow");
            arrows.clear();
            return false;
        }
    }
    return true;
}

bool ViewFileJSON::readArrow(std::vector<std::shared_ptr<Arrow>>& arrows, std::vector<std::shared_ptr<Node>>& nodes, const sajson::value& arrow) {
    // Make sure it's an object
    if(arrow.get_type() != TYPE_OBJECT) {
        SDL_Log("Error: arrow in JSON was not object type");
        return false;
    }
    // Checklist of things we need
    bool foundLabel = false;
    std::string label;
    bool foundSource = false;
    int sourceIndex;
    bool foundTarget = false;
    int targetIndex;
    bool foundX = false;
    float x;
    bool foundY = false;
    float y;
    auto length = arrow.get_length();
    for(auto i = 0u; i < length; i++) {
        auto key = arrow.get_object_key(i).as_string();
        auto val = arrow.get_object_value(i);
        if(key == "label") {
            if(val.get_type() == TYPE_STRING) {
                label = val.as_string();
                foundLabel = true;
            }
        }
        else if(key == "source") {
            if(val.get_type() == TYPE_INTEGER) {
                sourceIndex = val.get_number_value();
                foundSource = true;
            }
        }
        else if(key == "target") {
            if(val.get_type() == TYPE_INTEGER) {
                targetIndex = val.get_number_value();
                foundTarget = true;
            }
        }
        else if(key == "controlX") {
            if(val.get_type() == TYPE_DOUBLE || val.get_type() == TYPE_INTEGER) {
                x = val.get_number_value();
                foundX = true;
            }
        }
        else if(key == "controlY") {
            if(val.get_type() == TYPE_DOUBLE || val.get_type() == TYPE_INTEGER) {
                y = val.get_number_value();
                foundY = true;
            }
        }
    }
    // Put together arrow
    if(foundLabel && foundSource && foundTarget && foundX && foundY) {
        // Find source node
        std::shared_ptr<Node> sourceNode = nodes[sourceIndex];
        // Find target node
        std::shared_ptr<Node> targetNode = nodes[targetIndex];
        // Build control point
        SDL_FPoint controlPoint{x, y};
        // Create new arrow
        std::shared_ptr<Arrow> newArrow = std::shared_ptr<Arrow>(new Arrow(renderer, sourceNode, targetNode, label, controlPoint));
        // Register arrow with source and target nodes
        sourceNode->addOutgoingArrow(newArrow);
        targetNode->addIncomingArrow(newArrow);
        // Add to the list
        arrows.push_back(newArrow);
        return true;
    }
    SDL_Log("Error: not all components of arrow found in JSON.");
    return false;
}
