#pragma once

#include <SDL.h>
#include <string>
#include <vector>
#include "Renderable.h"
#include "TextLine.h"

class TextBox : public Renderable
{
public:
    TextBox(SDL_Renderer* renderer, int width, std::string text);

private:
    int width;
    std::vector<std::string> lines; // TODO consider only using lineTextures
    std::vector<std::shared_ptr<TextLine>> lineTextures;

    void _render(SDL_Renderer* renderer);
};
