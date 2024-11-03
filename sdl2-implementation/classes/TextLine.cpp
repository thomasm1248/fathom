#include "TextLine.h"

TextLine::TextLine(SDL_Renderer* renderer, TTF_Font* font, std::string text)
    : text(text)
    , Renderable(renderer)
{
    initializeTexture(98, 98);

    SDL_Color textColor{255, 255, 255, 255};
    auto surfacePtr = TTF_RenderUTF8_Blended(font, text.c_str(), textColor);
    if(!surfacePtr) {
        SDL_Log("Error: unable to create text surface.");
        return;
    }
    surface = std::shared_ptr<SDL_Surface>(surfacePtr, SDL_FreeSurface);
    surfacePtr = NULL;
}

void TextLine::_render(SDL_Renderer* renderer) {
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, &(*surface));
    if(!texture) {
        SDL_Log("Error: unable to create texture from surface.");
        return;
    }
    replaceTexture(texture);
    texture = NULL;
}
