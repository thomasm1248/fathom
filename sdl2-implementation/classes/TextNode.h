#pragma once

#include "Node.h"
#include <string>

class TextNode : public Node
{
public:
    TextNode(SDL_Renderer* renderer, std::string text);

private:
    std::string text;

    void _render(SDL_Renderer* renderer);
};
