#pragma once

#include "Renderable.h"
#include <string>
#include <memory>
#include <SDL_ttf.h>

class TextLine : public Renderable
{
public:
    bool wrapped = true;

    TextLine(SDL_Renderer* renderer, TTF_Font* font, const SDL_Point& location);
    int numCharacters();
    int indexAtXPos(int x);
    int xPosAtIndex(int index);
    std::string insertText(std::string newText, int index, int maxWidth);
    std::string insertNewline(int index);
    void moveLine(SDL_Point newLocation);
    void removeRange(int startIndex, int count);
    int pullTextFrom(std::shared_ptr<TextLine> other, int maxWidth);
    std::string getText();

private:
    std::string text;
    TTF_Font* font = NULL;
    std::shared_ptr<SDL_Surface> surface;

    void _render(SDL_Renderer* renderer);
};
