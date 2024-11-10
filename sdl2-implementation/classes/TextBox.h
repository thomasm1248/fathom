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
    TextBox(SDL_Renderer* renderer, TTF_Font* font, int width);
    void handleEvent(const SDL_Event& event);
    // Methods for parent to use
    void startEditing();
    void stopEditing();

private:
    int fontSize = 14;
    int margin = 5;
    int lineSpacing = 5;
    enum class State {
        Displaying,
        Editing,
        Selecting
    };
    State state = State::Displaying;
    int width;
    TTF_Font* font;
    std::vector<std::shared_ptr<TextLine>> lineTextures;
    SDL_Renderer* renderer;

    void _render(SDL_Renderer* renderer);
    void insertTextAtCursor(std::string text);
    void insertNewlineAtCursor();
    void handleKeypress(const SDL_Keysym &keysym);
    void doBackspaceAction();

    

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
