#include "TextNode.h"
#include <iostream>

TextNode::TextNode(SDL_Renderer* renderer, std::string text)
    : Node(renderer)
    , text(text)
{
    initializeTexture(100, 100);
}

void TextNode::_render(SDL_Renderer* renderer) {
    std::cout << "Rendering TextNode\n";
    SDL_Rect rect = getRect();
    std::cout << rect.w<<' '<<rect.h<<'\n';
    rect.x = 0;
    rect.y = 0;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);
}
