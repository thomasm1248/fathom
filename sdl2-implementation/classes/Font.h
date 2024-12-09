#pragma once

#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

class Font
{
public:
    Font(std::string filepath, int size, SDL_Color foreground, SDL_Color background);
    ~Font();
    TTF_Font* getFont();
    int getSize();
    int getHeight();
    SDL_Color getForeground();
    SDL_Color getBackground();

private:
    TTF_Font* font;
    int size;
    SDL_Color foreground;
    SDL_Color background;
};
