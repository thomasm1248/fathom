#include "TextBox.h"
#include <iostream>

TextBox::TextBox(SDL_Renderer* renderer, TTF_Font* font, int width, std::string text)
    : width(width)
    , font(font)
    , Renderable(renderer)
{
    // Break text into lines
    int lastBreak = 0;
    SDL_Point location;
    location.x = 5;
    location.y = 5;
    for(int i = 0; i < text.size(); i++) {
        if(text.at(i) == '\n') {
            // Newline found, create a line for the preceding text
            lineTextures.push_back(std::shared_ptr<TextLine>(new TextLine(renderer, font, text.substr(lastBreak, i-lastBreak), location)));
            lastBreak = i + 1;
            location.y += 14 + 5;
        }
    }
    // Create a line for the last bit of text too
    lineTextures.push_back(std::shared_ptr<TextLine>(new TextLine(renderer, font, text.substr(lastBreak, text.size()-lastBreak), location)));
    
    // Initialize texture
    initializeTexture(width, location.y + 14 + 5);
    moveTexture(1, 1);
}

void TextBox::handleEvent(const SDL_Event& event) {
    switch(event.type) {
    case SDL_MOUSEMOTION:
        SDL_Log("TextBox: mouse moved");
        break;
    case SDL_MOUSEBUTTONDOWN:
        SDL_Log("TextBox: mouse button down");
        break;
    case SDL_MOUSEBUTTONUP:
        SDL_Log("TextBox: mouse button up");
        break;
    case SDL_KEYDOWN:
        SDL_Log("TextBox: key down");
        break;
    case SDL_KEYUP:
        SDL_Log("TextBox: key up");
        break;
    case SDL_TEXTINPUT:
        SDL_Log("TextBox: text input");
        break;
    }
}

void TextBox::startEditing() {
    // TODO switchToStateEditing();
}

void TextBox::stopEditing() {
    switchToStateDisplaying();
}

void TextBox::_render(SDL_Renderer* renderer) {
    auto rect = getRect();
    rect.x = 0;
    rect.y = 0;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
    for(size_t i = 0; i < lineTextures.size(); i++) {
        drawChild(*lineTextures[i]);
    }
}

void TextBox::resetState() {
    switch(state) {
    case State::Displaying:
        break;
    }
}

void TextBox::switchToStateDisplaying() {
    resetState();
    state = State::Displaying;
    SDL_Log("TextBox State: Displaying");
    // This will probably change how this TextBox looks
    redrawRequested = true;
}
