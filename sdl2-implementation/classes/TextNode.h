#pragma once

#include "Node.h"
#include <string>
#include "TextBox.h"

class TextNode : public Node
{
public:
    TextNode(SDL_Renderer* renderer, std::string text);
    TextNode(SDL_Renderer* renderer, SDL_Point _position);
    ~TextNode();
    void startInteraction();
    void stopInteraction();
    void handleEvent(const SDL_Event& event);

private:
    static TTF_Font* font;
    static int numberOfTextNodes;
    std::shared_ptr<TextBox> textBox;
    bool interacting = false;

    void _render(SDL_Renderer* renderer);
};
