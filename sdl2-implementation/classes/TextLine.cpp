#include "TextLine.h"
#include <iostream>

TextLine::TextLine(SDL_Renderer* renderer, TTF_Font* font, std::string text, const SDL_Point& location)
    : text(text)
    , font(font)
    , Renderable(renderer)
{
    SDL_Color textColor{255, 255, 255, 255};
    auto surfacePtr = TTF_RenderUTF8_Blended(font, text.c_str(), textColor);
    if(!surfacePtr) {
        SDL_Log("Error: unable to create text surface.");
        return;
    }
    initializeTexture(surfacePtr->w, surfacePtr->h);
    moveTexture(location.x, location.y);
    surface = std::shared_ptr<SDL_Surface>(surfacePtr, SDL_FreeSurface);
    surfacePtr = NULL;
}

int TextLine::numCharacters() {
    return text.size();
}

int TextLine::indexAtXPos(int x) {
    SDL_Log("Error: TextLine::indexAtXPos is not defined");
    return 0;
}

int TextLine::xPosAtIndex(int index) {
    std::string textUpToIndex = text.substr(0, index);
    int width = 0;
    if(TTF_MeasureUTF8(font, text.c_str(), std::numeric_limits<int>::max(), &width, NULL) == -1) {
        SDL_Log("Error: unable to measure text in TextLine::xPosAtIndex");
    }
    return width;
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
