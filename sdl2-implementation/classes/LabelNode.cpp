#include "LabelNode.h"
#include "Util.h"
#include <sstream>
#include <iostream>

LabelNode::LabelNode(SDL_Renderer* renderer, std::shared_ptr<Font> font, SDL_Point location)
    : Node(renderer)
    , renderer(renderer)
    , font(font)
    , lineSpacing(font->getHeight() * 0.1)
    , fontSize(font->getHeight())
{
    initializeTexture(1, 1);
    moveTexture(location.x, location.y);
    SDL_Point lineLocation{0, 0}; // It doesn't matter what this is
    lines.push_back(std::make_shared<TextLine>(renderer, font->getFont(), lineLocation));
    arrange();
}

LabelNode::LabelNode(SDL_Renderer* renderer, std::shared_ptr<Font> font, SDL_Point location, std::string text)
    : Node(renderer)
    , renderer(renderer)
    , font(font)
    , lineSpacing(font->getHeight() * 0.1)
    , fontSize(font->getHeight())
{
    initializeTexture(1, 1);
    moveTexture(location.x, location.y);
    SDL_Point lineLocation{0, 0}; // It doesn't matter what this is
    lines.push_back(std::make_shared<TextLine>(renderer, font->getFont(), lineLocation));
    auto lines = Util::splitIntoLines(std::move(text));
    for(size_t i = 0; i < lines.size(); i++) {
        if(i != 0) insertNewline();
        insertText(lines[i]);
    }
}

void LabelNode::handleEvent(const SDL_Event& event) {
    if(!interacting) {
        SDL_Log("Error: LabelNode::handleEvent called when not being interacted with.");
        return;
    }
    switch(event.type) {
    case SDL_KEYDOWN:
        if(event.key.keysym.sym == SDLK_BACKSPACE) {
            backspace();
        }
        else if(event.key.keysym.sym == SDLK_RETURN) {
            insertNewline();
        }
        break;
    case SDL_TEXTINPUT:
        std::string inputText = std::string(event.text.text);
        insertText(inputText);
        break;
    }
}

void LabelNode::startInteraction() {
    SDL_Log("Started interaction");
    interacting = true;
    redrawRequested = true; // TODO NOW: do I even need this?
}

void LabelNode::stopInteraction() {
    SDL_Log("Stopped interaction");
    interacting = false;
    redrawRequested = true; // TODO NOW: do I even need this?
}

std::string LabelNode::toString() {
    std::stringstream ss;
    // Get position
    auto rect = getRect();
    // Get text
    auto text = getText();
    Util::replace_all(text, "\n", "\\n");
    Util::replace_all(text, "\"", "\\\"");
    // Serialize
    ss << "{";
    ss << "\"x\": " << (rect.x + rect.w/2) << ", ";
    ss << "\"y\": " << (rect.y + rect.h/2) << ", ";
    ss << "\"label\": \"" << text << "\"";
    ss << "}";
    return ss.str();
}

void LabelNode::_render(SDL_Renderer* renderer) {
    // Draw background
    auto backgroundColor = font->getBackground();
    if(isSelected() || isHovered()) {
        backgroundColor.r = 30;
        backgroundColor.g = 30;
        backgroundColor.b = 30;
    }
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_Rect rect = getRect();
    rect.x = 0;
    rect.y = 0;
    SDL_RenderFillRect(renderer, &rect);
    // Draw lines of text
    for(size_t i = 0; i < lines.size(); i++) {
        drawChild(*lines[i]);
    }
}

void LabelNode::arrange() {
    // Record current rect
    auto oldRect = getRect();
    // Find the widest line
    int maxWidth = 0;
    for(size_t i = 0; i < lines.size(); i++) {
        int sizeOfThisLine = lines[i]->getRect().w;
        if(sizeOfThisLine > maxWidth)
            maxWidth = sizeOfThisLine;
    }
    // Re-position all the lines
    SDL_Point lineLocation{0, margin};
    for(size_t i = 0; i < lines.size(); i++) {
        int sizeOfThisLine = lines[i]->getRect().w;
        lineLocation.x = (maxWidth - sizeOfThisLine) / 2 + margin;
        lines[i]->moveLine(lineLocation);
        lineLocation.y += lineSpacing + fontSize;
    }
    // Resize texture
    createOverlapRect();
    resizeTexture(maxWidth + margin*2, lines.size() * (lineSpacing + fontSize) - lineSpacing + margin*2);
    // Move texture so it's centered over the center of its old rect
    SDL_Rect currentRect = getRect();
    moveTexture(oldRect.x + oldRect.w/2 - currentRect.w/2, oldRect.y + oldRect.h/2 - currentRect.h/2);
}

void LabelNode::insertText(std::string text) {
    auto line = lines[lines.size()-1]; // Get the last line
    line->insertText(text, line->numCharacters(), 2000000000);
    arrange();
}

void LabelNode::insertNewline() {
    SDL_Point lineLocation{0, 0}; // It doesn't matter what this is
    lines.push_back(std::make_shared<TextLine>(renderer, font->getFont(), lineLocation));
    arrange();
}

void LabelNode::backspace() {
    auto line = lines[lines.size()-1]; // Get last line
    if(line->numCharacters() == 0) {
        // Remove last line
        lines.erase(lines.end() - 1);
    }
    else {
        // Remove last character of last line
        std::string text = line->getText();
        text = text.substr(0, text.size()-1); // Remove last character
        SDL_Point lineLocation{0, 0}; // It doesn't matter what this is
        line = std::make_shared<TextLine>(renderer, font->getFont(), lineLocation);
        line->insertText(text, 0, 2000000000);
        lines[lines.size()-1] = line;
    }
    arrange();
}

void LabelNode::_selectedStatusHasChanged(bool isItSelected) {
    redrawRequested = true;
}

void LabelNode::_hoveredStatusHasChanged(bool isItHovered) {
    redrawRequested = true;
}

std::string LabelNode::getText() {
    std::stringstream ss;
    for(size_t i = 0; i < lines.size(); i++) {
        if(i != 0) ss << '\n';
        ss << lines[i]->getText();
    }
    return ss.str();
}

void LabelNode::reset() {
    // Delete current lines
    lines.clear();
    // Create a new line
    SDL_Point lineLocation{0, 0}; // It doesn't matter what this is
    lines.push_back(std::make_shared<TextLine>(renderer, font->getFont(), lineLocation));
    arrange();
}
