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
            handleKeypress(event.key.keysym);
            break;
        case SDL_KEYUP:
            SDL_Log("TextBox: key up");
            break;
        case SDL_TEXTINPUT:
            std::string inputText = std::string(event.text.text);
            insertTextAtCursor(inputText);
            _rememberedCursorHeight = -1;
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
    // TODO implement word-wrap and newlines
    // Insert text into a line, and add the overflow to the next line...
    int insertIndex = _characterIndexOfCursor; // insert at current cursor
    _characterIndexOfCursor += text.size();
    int lastLineWrapped;
    int lastLineWrappedEndsInNewline = false;
    for(size_t i = _lineIndexOfCursor; i < lineTextures.size(); i++) {
        lastLineWrapped = i;
        // Check if this line ends with a newline
        bool thisLineEndsWithNewline = !lineTextures[i]->wrapped;
        // Insert the text, taking the extra off the end
        text = lineTextures[i]->insertText(text, insertIndex, width - margin*2);
        // Insert at the beggining of all future lines instead
        insertIndex = 0;
        // Update the cursor position
        if(_characterIndexOfCursor > lineTextures[i]->numCharacters()) {
            // Move cursor to next line
            _lineIndexOfCursor++;
            _characterIndexOfCursor -= lineTextures[i]->numCharacters();
        }
        // If this line didn't need to be wrapped, then we're done
        if(text.size() == 0) {
            redrawRequested = true;
            return;
        }
        // If this line ends with newline, stop wrapping to following lines
        else if(thisLineEndsWithNewline) {
            lastLineWrappedEndsInNewline = true;
            break;
        }
    }
    /* If we've reached this point then one of the following are true:
        * New lines need to be created at the end of the textbox to
          accommodate the remaining text
        * New lines need to be created to accommodate the remaining
          text, and the new lines need to be inserted in-between
          existing lines, pushing following lines down
    */
    // Prepare to create new lines for additional overflow
    auto rectOfLastLineWrapped = lineTextures[lastLineWrapped]->getRect();
    SDL_Point newLineLocation;
    newLineLocation.x = rectOfLastLineWrapped.x;
    newLineLocation.y = rectOfLastLineWrapped.y;
    int initialNumberOfLines = lineTextures.size();
    // If there's any more overflow, divide it between additional lines
    int newLinesCreated = 0;
    while(text.size() > 0) {
        // Calculate position of new line
        newLineLocation.y += fontSize + lineSpacing;
        // Create new line, and give it as much of the extra text as it will take
        auto newLine = std::make_shared<TextLine>(renderer, font, newLineLocation);
        text = newLine->insertText(text, 0, width - margin*2);
        // Insert the new line into the list
        lineTextures.insert(lineTextures.begin() + lastLineWrapped + ++newLinesCreated, newLine);
    }
    // Make sure last created line is recorded as ending in a newline
    lineTextures[lastLineWrapped + newLinesCreated]->wrapped = !lastLineWrappedEndsInNewline;
    // If new lines were inserted before existing lines, push following lines down
    for(int i = lastLineWrapped + 1 + newLinesCreated; i < lineTextures.size(); i++) {
        // Calculate new location of line
        newLineLocation.y += fontSize + lineSpacing;
        // Move line
        lineTextures[i]->moveLine(newLineLocation);
    }
    // Update redraw status and dimensions to accommodate new lines
    redrawRequested = true;
    int height = margin * 2 + (lineTextures.size()-1) * (fontSize+lineSpacing) + fontSize;
    resizeTexture(width, height);
}

void TextBox::insertNewlineAtCursor() {
    // Split current line at cursor
    bool lineWasWrapped = lineTextures[_lineIndexOfCursor]->wrapped;
    std::string newLineText = lineTextures[_lineIndexOfCursor]->insertNewline(_characterIndexOfCursor);
    // Update the cursor position
    _lineIndexOfCursor++;
    _characterIndexOfCursor = 0;
    // Handle following lines
    if(lineWasWrapped) {
        // If the cursor was on the last line, move extra text to a new line
        if(_lineIndexOfCursor == lineTextures.size()) {
            // Calculate position for new line
            SDL_Point lineLocation;
            lineLocation.x = margin;
            lineLocation.y = margin + _lineIndexOfCursor * (fontSize+lineSpacing);
            // Create a new line for the extra text
            auto newLine = std::make_shared<TextLine>(renderer, font, lineLocation);
            lineTextures.push_back(newLine);
            // Resize texture to accommodate extra line
            resizeTexture(width, margin*2 + fontSize + (lineTextures.size()-1)*(fontSize+lineSpacing));
        }
        // Otherwise, insert extra text into the next line, and reset the cursor
        // to the beginning
        else {
            insertTextAtCursor(newLineText);
            _characterIndexOfCursor = 0;
        }
    }
    else {
        // Calculate position for new line
        SDL_Point lineLocation;
        lineLocation.x = margin;
        lineLocation.y = margin + _lineIndexOfCursor * (fontSize+lineSpacing);
        // Create a new line for the extra text
        auto newLine = std::make_shared<TextLine>(renderer, font, lineLocation);
        lineTextures.insert(lineTextures.begin() + _lineIndexOfCursor, newLine);
        // Push the following lines down
        for(int i = _lineIndexOfCursor + 1; i < lineTextures.size(); i++) {
            // Calculate new position of line
            lineLocation.y += fontSize + lineSpacing;
            // Move line
            lineTextures[i]->moveLine(lineLocation);
        }
        // Resize texture to accommodate extra line
        resizeTexture(width, margin*2 + fontSize + (lineTextures.size()-1)*(fontSize+lineSpacing));
    }
    // Make sure the textbox is re-rendered
    redrawRequested = true;
}

void TextBox::handleKeypress(const SDL_Keysym &keysym) {
    // Assume that this function was called during editing state
    if(state != State::Editing) {
        SDL_Log("Error: TextBox::handleKeypress called outside of Editing state");
        return;
    }
    switch(keysym.sym) {
    case SDLK_BACKSPACE:
        doBackspaceAction();
        _rememberedCursorHeight = -1;
        break;
    case SDLK_RETURN:
        insertNewlineAtCursor();
        _rememberedCursorHeight = -1;
        break;
    case SDLK_RIGHT:
        _characterIndexOfCursor++;
        if(_characterIndexOfCursor > lineTextures[_lineIndexOfCursor]->numCharacters()) {
            if(_lineIndexOfCursor < lineTextures.size()-1) {
                _lineIndexOfCursor++;
                _characterIndexOfCursor = 0;
            }
            else {
                _characterIndexOfCursor--;
            }
        }
        _rememberedCursorHeight = -1;
        redrawRequested = true;
        break;
    case SDLK_LEFT:
        _characterIndexOfCursor--;
        if(_characterIndexOfCursor < 0) {
            if(_lineIndexOfCursor > 0) {
                _lineIndexOfCursor--;
                _characterIndexOfCursor = lineTextures[_lineIndexOfCursor]->numCharacters();
            }
            else {
                _characterIndexOfCursor++;
            }
        }
        _rememberedCursorHeight = -1;
        redrawRequested = true;
        break;
    case SDLK_UP:
        if(_lineIndexOfCursor == 0) {
            _characterIndexOfCursor = 0;
            _rememberedCursorHeight = -1;
        } else {
            _lineIndexOfCursor--;
            if(_rememberedCursorHeight > -1) {
                _characterIndexOfCursor = _rememberedCursorHeight;
            }
            else {
                _rememberedCursorHeight = _characterIndexOfCursor;
            }
            int heightOfCurrentLine = lineTextures[_lineIndexOfCursor]->numCharacters();
            if(_characterIndexOfCursor > heightOfCurrentLine) {
                _characterIndexOfCursor = heightOfCurrentLine;
            }
        }
        redrawRequested = true;
        break;
    case SDLK_DOWN:
        if(_lineIndexOfCursor == lineTextures.size() - 1) {
            _characterIndexOfCursor = lineTextures[lineTextures.size()-1]->numCharacters();
            _rememberedCursorHeight = -1;
        }
        else {
            _lineIndexOfCursor++;
            if(_rememberedCursorHeight > -1) {
                _characterIndexOfCursor = _rememberedCursorHeight;
            }
            else {
                _rememberedCursorHeight = _characterIndexOfCursor;
            }
            int heightOfCurrentLine = lineTextures[_lineIndexOfCursor]->numCharacters();
            if(_characterIndexOfCursor > heightOfCurrentLine) {
                _characterIndexOfCursor = heightOfCurrentLine;
            }
        }
        redrawRequested = true;
        break;
    }
}

void TextBox::doBackspaceAction() {
    // TODO
}

void TextBox::resetState() {
    switch(state) {
    case State::Displaying:
        break;
    case State::Editing:
        _lineIndexOfCursor = 0;
        _characterIndexOfCursor = 0;
        _rememberedCursorHeight = -1;
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
