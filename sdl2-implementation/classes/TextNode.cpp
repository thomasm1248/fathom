#include "TextNode.h"
#include <iostream>

TTF_Font* TextNode::font = NULL;
int TextNode::numberOfTextNodes = 0;

TextNode::TextNode(SDL_Renderer* renderer, std::string text)
    : Node(renderer)
{
    numberOfTextNodes++;
    initializeTexture(100, 100);

    // Initialize font if not done already
    if(!font) {
        font = TTF_OpenFont("AovelSansRounded-rdDL.ttf", 14);
        if(!font) {
            SDL_Log("Error: unable to open font.");
            std::cout << TTF_GetError() << '\n';
            return;
        }
    }
    
    // Initialize textbox
    textBox = std::make_shared<TextBox>(renderer, font, 98, text);
}

TextNode::TextNode(SDL_Renderer* renderer)
    : Node(renderer)
{
    numberOfTextNodes++;
    initializeTexture(100, 100);

    // Initialize font if not done already
    if(!font) {
        font = TTF_OpenFont("AovelSansRounded-rdDL.ttf", 14);
        if(!font) {
            SDL_Log("Error: unable to open font.");
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
    SDL_Log("Started interaction");
    interacting = true;
    textBox->startEditing();
    redrawRequested = true;
}

void TextNode::stopInteraction() {
    SDL_Log("Stopped interaction");
    interacting = false;
    textBox->stopEditing();
    redrawRequested = true;
}

void TextNode::handleEvent(const SDL_Event& event) {
    textBox->handleEvent(event);
    if(textBox->requestsToBeRedrawn()) redrawRequested = true;
}

void TextNode::_render(SDL_Renderer* renderer) {
    SDL_Rect rect = getRect();
    rect.x = 0;
    rect.y = 0;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);
    drawChild(*textBox);
}
