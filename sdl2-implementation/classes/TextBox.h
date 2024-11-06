#pragma once

#include <SDL.h>
#include <string>
#include <vector>
#include "Renderable.h"
#include "TextLine.h"

class TextBox : public Renderable
{
public:
    TextBox(SDL_Renderer* renderer, TTF_Font* font, int width, std::string text);
    void handleEvent(const SDL_Event& event);
    // Methods for parent to use
    void startEditing();
    void stopEditing();

private:
    enum class State {
        Displaying,
        Editing,
        Selecting
    };
    State state = State::Displaying;
    int width;
    TTF_Font* font;
    std::vector<std::shared_ptr<TextLine>> lineTextures;

    void _render(SDL_Renderer* renderer);

    

    // State Machine

    // shared
    void resetState();

    // Displaying
    void switchToStateDisplaying();

    // Editing
    void switchToStateEditing();
    int _lineIndexOfCursor;
    int _characterIndexOfCursor;
};
