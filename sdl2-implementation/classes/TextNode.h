#pragma once

#include "Node.h"
#include <string>
#include "TextBox.h"

class TextNode : public Node
{
public:
    TextNode(SDL_Renderer* renderer, std::string text);
    ~TextNode();

private:
    static TTF_Font* font;
    static int numberOfTextNodes;
    std::shared_ptr<TextBox> textBox;

    void _render(SDL_Renderer* renderer);
};
