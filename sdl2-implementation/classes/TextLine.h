#pragma once

#include "Renderable.h"
#include <string>
#include <memory>
#include <SDL_ttf.h>

class TextLine : public Renderable
{
public:
    bool wrapped = false;

    TextLine(SDL_Renderer* renderer, TTF_Font* font, std::string text, const SDL_Point& location);
    int numCharacters();
    int indexAtXPos(int x);
    int xPosAtIndex(int index);

private:
    std::string text;
    TTF_Font* font = NULL;
    std::shared_ptr<SDL_Surface> surface;

    void _render(SDL_Renderer* renderer);
};
