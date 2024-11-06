#include "TextBox.h"
#include <iostream>

TextBox::TextBox(SDL_Renderer* renderer, TTF_Font* font, int width, std::string text)
    : width(width)
    , font(font)
    , Renderable(renderer)
    , renderer(renderer)
{
    // Initialize texture
    initializeTexture(1, 1); // placeholder
    moveTexture(1, 1); // TODO take a specified position
    // Break text into lines
    SDL_Point startLocation;
    startLocation.x = 0;
    startLocation.y = 0;
    lineTextures.push_back(std::shared_ptr<TextLine>(new TextLine(renderer, font, startLocation)));
    insertTextAtCursor(text);
}

TextBox::TextBox(SDL_Renderer* renderer, TTF_Font* font, int width)
    : width(width)
    , font(font)
    , Renderable(renderer)
    , renderer(renderer)
{
    // Initialize texture
    initializeTexture(width, margin*2+fontSize);
    moveTexture(1, 1); // TODO take a specified position
    // Break text into lines
    SDL_Point startLocation;
    startLocation.x = margin;
    startLocation.y = margin;
    lineTextures.push_back(std::shared_ptr<TextLine>(new TextLine(renderer, font, startLocation)));
}

void TextBox::handleEvent(const SDL_Event& event) {
    if(state == State::Editing) {
        switch(event.type) {
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
            std::string inputText = std::string(event.text.text);
            insertTextAtCursor(inputText);
            break;
        }
    }
}

void TextBox::startEditing() {
    switchToStateEditing();
}

void TextBox::stopEditing() {
    switchToStateDisplaying();
}

void TextBox::_render(SDL_Renderer* renderer) {
    auto rect = getRect();
    rect.x = 0;
    rect.y = 0;
    // Draw background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
    // Draw lines of text
    for(size_t i = 0; i < lineTextures.size(); i++) {
        drawChild(*lineTextures[i]);
    }
    // Draw cursor
    if(state == State::Editing) {
        // Find position of cursor
        SDL_Rect cursorRect;
        cursorRect.x = margin + lineTextures[_lineIndexOfCursor]->xPosAtIndex(_characterIndexOfCursor);
        cursorRect.y = margin + _lineIndexOfCursor * (fontSize+lineSpacing);
        // Draw cursor
        cursorRect.w = 1;
        cursorRect.h = fontSize;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &cursorRect);
    }
}

void TextBox::insertTextAtCursor(std::string text) {
    // TODO implement word-wrap
    // Insert text into a line, and add the overflow to the next line...
    int insertIndex = _characterIndexOfCursor; // insert at current cursor
    for(size_t i = _lineIndexOfCursor; i < lineTextures.size(); i++) {
        _characterIndexOfCursor += text.size();
        text = lineTextures[i]->insertText(text, insertIndex, width - margin*2);
        insertIndex = 0; // insert at the beggining of all future lines instead
        if(_characterIndexOfCursor > lineTextures[i]->numCharacters()) {
            // Move cursor to next line
            _lineIndexOfCursor++;
            _characterIndexOfCursor -= lineTextures[i]->numCharacters();
        }
        if(text.size() == 0) break;
    }
    // Prepare to create new lines for additional overflow
    auto lastLineRect = lineTextures[lineTextures.size()-1]->getRect();
    SDL_Point newLineLocation;
    newLineLocation.x = lastLineRect.x;
    newLineLocation.y = lastLineRect.y;
    int initialNumberOfLines = lineTextures.size();
    // If there's any more overflow, divide it between additional lines
    while(text.size() > 0) {
        newLineLocation.y += fontSize + lineSpacing;
        auto newLine = std::make_shared<TextLine>(renderer, font, newLineLocation);
        text = newLine->insertText(text, 0, width - margin*2);
        lineTextures.push_back(newLine);
    }
    // Update redraw status and dimensions
    redrawRequested = true;
    if(lineTextures.size() != initialNumberOfLines) {
        // Number of lines has changed, so the size must as well
        int height = margin * 2 + (lineTextures.size()-1) * (fontSize+lineSpacing) + fontSize;
        resizeTexture(width, height);
    }
}

void TextBox::resetState() {
    switch(state) {
    case State::Displaying:
        break;
    case State::Editing:
        _lineIndexOfCursor = 0;
        _characterIndexOfCursor = 0;
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

void TextBox::switchToStateEditing() {
    resetState();
    state = State::Editing;
    SDL_Log("TextBox State: Editing");
    // Place cursor at the end of the text
    _lineIndexOfCursor = lineTextures.size() - 1;
    _characterIndexOfCursor = lineTextures[_lineIndexOfCursor]->numCharacters();
    redrawRequested = true;
}
