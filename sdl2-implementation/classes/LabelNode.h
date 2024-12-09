#pragma once

#include "Node.h"
#include <string>
#include <vector>
#include <memory>
#include "TextLine.h"
#include "Font.h"

class LabelNode : public Node
{
public:
    LabelNode(SDL_Renderer* renderer, std::shared_ptr<Font> font, SDL_Point location);
    LabelNode(SDL_Renderer* renderer, std::shared_ptr<Font> font, SDL_Point location, std::string text);
    void handleEvent(const SDL_Event& event);
    void startInteraction();
    void stopInteraction();
    std::string toString();

private:
    SDL_Renderer* renderer;
    std::shared_ptr<Font> font;
    std::vector<std::shared_ptr<TextLine>> lines;
    SDL_Rect overlapRect;
    bool hasOverlapRect = false;
    static constexpr int margin = 5;
    int lineSpacing;
    int fontSize;
    bool interacting = false;

    void _render(SDL_Renderer* renderer);
    void arrange();
    void insertText(std::string text);
    void insertNewline();
    void backspace();
    void _selectedStatusHasChanged(bool isItSelected);
    void _hoveredStatusHasChanged(bool isItHovered);
    std::string getText();
    void reset();
};
