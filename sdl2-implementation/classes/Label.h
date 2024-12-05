#pragma once

#include "Renderable.h"
#include <string>
#include <vector>
#include <memory>
#include "TextLine.h"
#include "Font.h"

class Label : public Renderable
{
public:
    Label(SDL_Renderer* renderer, std::shared_ptr<Font> font, SDL_Point location);
    Label(SDL_Renderer* renderer, std::shared_ptr<Font> font, SDL_Point location, std::string text);
    void handleEvent(SDL_Event& event);
    void setPosition(SDL_Point newLocation);
    void reset();
    bool getOverlapRect(SDL_Rect& rect);

private:
    SDL_Renderer* renderer;
    std::shared_ptr<Font> font;
    std::vector<std::shared_ptr<TextLine>> lines;
    SDL_Rect overlapRect;
    bool hasOverlapRect = false;
    static constexpr int margin = 5;
    int lineSpacing;
    int fontSize;
    SDL_Point location;

    void _render(SDL_Renderer* renderer);
    void arrange();
    void insertText(std::string text);
    void insertNewline();
    void backspace();
};
