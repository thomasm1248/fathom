#include "TextBox.h"
#include <iostream>

TextBox::TextBox(SDL_Renderer* renderer, int width, std::string text)
    : width(width)
    , Renderable(renderer)
{
    initializeTexture(98, 98);
    moveTexture(1, 1);

    // Break text into lines
    int lastBreak = 0;
    for(int i = 0; i < text.size(); i++) {
        if(text.at(i) == '\n') {
            // Newline found
            lines.push_back(text.substr(lastBreak, i-lastBreak));
            lastBreak = i + 1;
        }
    }
    lines.push_back(text.substr(lastBreak, text.size()-lastBreak));

    // Text:
    TTF_Init(); // TODO call TTF_Quit()
    auto font = TTF_OpenFont("AovelSansRounded-rdDL.ttf", 12);
    if(!font) {
        SDL_Log("Error: unable to open font.");
        std::cout << TTF_GetError() << '\n';
        return;
    }
    // TODO call TTF_CloseFont(font)
    lineTextures.push_back(std::shared_ptr<TextLine>(new TextLine(renderer, font, "ontehu")));
}

void TextBox::_render(SDL_Renderer* renderer) {
    auto rect = getRect();
    rect.x = 0;
    rect.y = 0;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
    for(size_t i = 0; i < lineTextures.size(); i++) {
        drawChild(*lineTextures[i]);
    }
}
