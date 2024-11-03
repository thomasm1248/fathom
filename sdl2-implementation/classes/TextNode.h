#pragma once

#include "Node.h"
#include <string>
#include "TextBox.h"

class TextNode : public Node
{
public:
    TextNode(SDL_Renderer* renderer, std::string text);

private:
    std::string text;
    TextBox textBox;

    void _render(SDL_Renderer* renderer);
};
