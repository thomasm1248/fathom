#include "TextNode.h"
#include <iostream>
#include <sstream>
#include "Util.h"

TTF_Font* TextNode::font = NULL;
int TextNode::numberOfTextNodes = 0;

TextNode::TextNode(SDL_Renderer* renderer, std::string text, SDL_Point _position)
    : Node(renderer)
{
    numberOfTextNodes++;

    // Initialize font if not done already
    if(!font) {
        font = TTF_OpenFont("Aovel Sans Rounded.ttf", 14);
        if(!font) {
            SDL_Log("Error: TextNode was unable to open font.");
            std::cout << TTF_GetError() << '\n';
            return;
        }
    }
    
    // Initialize textbox
    textBox = std::make_shared<TextBox>(renderer, font, 98, text);
    // Initialize texture
    SDL_Rect textBoxRect = textBox->getRect();
    initializeTexture(textBoxRect.w + 2, textBoxRect.h + 2);
    moveTexture(_position.x, _position.y);
}

TextNode::TextNode(SDL_Renderer* renderer, SDL_Point _position)
    : Node(renderer)
{
    numberOfTextNodes++;
    initializeTexture(100, 26);
    moveTexture(_position.x, _position.y);

    // Initialize font if not done already
    if(!font) {
        font = TTF_OpenFont("Aovel Sans Rounded.ttf", 14);
        if(!font) {
            SDL_Log("Error: TextNode was unable to open font.");
            std::cout << TTF_GetError() << '\n';
            return;
        }
    }
    
    // Initialize textbox
    textBox = std::make_shared<TextBox>(renderer, font, 98);
}

TextNode::~TextNode() {
    numberOfTextNodes--;
    if(numberOfTextNodes == 0) {
        TTF_CloseFont(font);
        font = NULL;
    }
}

void TextNode::startInteraction() {
    interacting = true;
    textBox->startEditing();
    // Resize node to match textBox
    auto rect = textBox->getRect();
    createOverlapRect();
    resizeTexture(rect.w + 2, rect.h + 2);
}

void TextNode::stopInteraction() {
    interacting = false;
    textBox->stopEditing();
    // Resize node to match textBox
    auto rect = textBox->getRect();
    createOverlapRect();
    resizeTexture(rect.w + 2, rect.h + 2);
}

void TextNode::handleEvent(const SDL_Event& event) {
    // It should only receive events during interaction
    if(!interacting) {
        SDL_Log("Error: TextNode::handleEvent called outside of interaction");
        return;
    }
    SDL_Rect oldTextBoxRect = textBox->getRect();
    textBox->handleEvent(event);
    if(textBox->requestsToBeRedrawn()) {
        redrawRequested = true;
        // Resize node to fit textbox if needed
        SDL_Rect newTextBoxRect = textBox->getRect();
        if(newTextBoxRect.w != oldTextBoxRect.w ||
           newTextBoxRect.h != oldTextBoxRect.h
        ) {
            createOverlapRect();
            resizeTexture(newTextBoxRect.w+2, newTextBoxRect.h+2);
        }
    }
}

std::string TextNode::toString() {
    std::stringstream ss;
    // Get position
    auto rect = getRect();
    // Get text
    auto text = textBox->getText();
    Util::replace_all(text, "\n", "\\n");
    Util::replace_all(text, "\"", "\\\"");
    // Serialize
    ss << "{";
    ss << "\"x\": " << rect.x << ", ";
    ss << "\"y\": " << rect.y << ", ";
    ss << "\"text\": \"" << text << "\"";
    ss << "}";
    return ss.str();
}

bool TextNode::isEmpty() {
    return textBox->getText().size() == 0;
}

void TextNode::_render(SDL_Renderer* renderer) {
    SDL_Rect rect = getRect();
    rect.x = 0;
    rect.y = 0;
    if(interacting || isSelected() || isHovered()) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    }
    SDL_RenderFillRect(renderer, &rect);
    drawChild(*textBox);
}

void TextNode::_selectedStatusHasChanged(bool isItSelected) {
    redrawRequested = true;
}

void TextNode::_hoveredStatusHasChanged(bool isItHovered) {
    redrawRequested = true;
}
