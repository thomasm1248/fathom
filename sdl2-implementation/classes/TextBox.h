#pragma once

#include <SDL.h>
#include <string>
#include <vector>

class TextBox
{
public:
    TextBox() {}
    TextBox(std::string text);

private:
    std::vector<std::string> lines;
    //SDL_Texture cachedTexture
};
