#include "Font.h"
#include <iostream>

Font::Font(std::string filepath, int size, SDL_Color foreground, SDL_Color background)
    : size(size)
    , foreground(foreground)
    , background(background)
{
    // Set up the font
    font = TTF_OpenFont(filepath.c_str(), size);
    if(!font) {
        SDL_Log("Error: unable to open font.");
        std::cout << TTF_GetError() << '\n';
        return;
    }
}

Font::~Font() {
    // Destroy font
    if(font) {
        TTF_CloseFont(font);
        font = NULL;
    }
}

TTF_Font* Font::getFont() {
    return font;
}

int Font::getSize() {
    return size;
}

int Font::getHeight() {
    return TTF_FontHeight(font);
}

SDL_Color Font::getForeground() {
    return foreground;
}

SDL_Color Font::getBackground() {
    return background;
}
