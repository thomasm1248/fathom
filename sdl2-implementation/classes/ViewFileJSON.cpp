#include "ViewFileJSON.h"
#include <iostream>
#include <errno.h>
#include "TextNode.h"
#include <fstream>

using namespace sajson;

inline bool success(const document& doc) {
    if (!doc.is_valid()) {
        fprintf(stderr, "%s\n", doc.get_error_message_as_cstring());
        return false;
    }
    return true;
}

ViewFileJSON::ViewFileJSON(std::string filepath, SDL_Renderer* renderer)
    : filepath(filepath)
    , renderer(renderer)
{
}

bool ViewFileJSON::read(std::vector<std::shared_ptr<Node>> &nodes) {
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
    traverse(nodes, document.get_root());

    delete[] buffer;

    return true;
}

bool ViewFileJSON::write(std::vector<std::shared_ptr<Node>> &nodes) {
    std::ofstream file;
    file.open(filepath);
    file << "{\"version\": 0, \"nodes\": [";
    for(size_t i = 0; i < nodes.size(); i++) {
        if(i != 0) file << ", ";
        file << "{\"id\": " << i << ", ";
        auto rect = nodes[i]->getRect();
        file << "\"x\": " << rect.x << ", ";
        file << "\"y\": " << rect.y << ", ";
        file << "\"type\": 0, ";
        auto content = nodes[i]->getContent();
        file << "\"content\": \"" << content << "\"}";
    }
    file << "]}\n";
    file.close();
    return true;
}
    
bool ViewFileJSON::traverse(std::vector<std::shared_ptr<Node>> &nodes, const sajson::value& view) {
    // Make sure the file has an object
    if(view.get_type() != TYPE_OBJECT) {
        SDL_Log("Error: root JSON object was not object type");
        return false;
    }
    // Iterate through properties
    auto length = view.get_length();
    bool nodeListRead = false;
    for(auto i = 0u; i < length; i++) {
        auto key = view.get_object_key(i).as_string();
        if(key == "nodes") {
            if(!readNodeList(nodes, view.get_object_value(i))) {
                SDL_Log("Error: unable to read node list");
                return false;
            }
            nodeListRead = true;
        }
    }
    if(!nodeListRead) {
        SDL_Log("Error: node list not found in file");
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
    bool foundContent = false;
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
        else if(key == "content") {
            if(val.get_type() == TYPE_STRING) {
                content = val.as_string();
                foundContent = true;
            }
        }
    }
    // Put together node
    if(foundX && foundY && foundContent) {
        // Make node
        SDL_Point location;
        location.x = x;
        location.y = y;
        auto newNode = std::shared_ptr<TextNode>(new TextNode(renderer, content, location));
        nodes.push_back(newNode);
        SDL_Log("pushed a node");
        return true;
    }
    return false;
}
