#include "TextBox.h"
#include <iostream>
#include <sstream>

TextBox::TextBox(SDL_Renderer* renderer, TTF_Font* font, int width, std::string text)
    : width(width)
    , font(font)
    , Renderable(renderer)
    , renderer(renderer)
{
    // Initialize texture
    initializeTexture(width, margin*2+fontSize);
    moveTexture(1, 1); // TODO take a specified position
    // Inilialize first line
    SDL_Point startLocation;
    startLocation.x = margin;
    startLocation.y = margin;
    lineTextures.push_back(std::shared_ptr<TextLine>(new TextLine(renderer, font, startLocation)));
    // Break text into lines
    std::vector<std::string> strings;
    std::string delimiter = "\n";
    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = text.find(delimiter, prev)) != std::string::npos)
    {
        strings.push_back(text.substr(prev, pos - prev));
        prev = pos + delimiter.size();
    }
    strings.push_back(text.substr(prev));
    // Insert each line into the textbox one-by-one
    startEditing();
    for(size_t i = 0; i < strings.size(); i++) {
        if(i != 0) insertNewlineAtCursor();
        insertTextAtCursor(strings[i]);
    }
    stopEditing();
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
            if(event.button.button == SDL_BUTTON_LEFT) {
                // Find local position of mouse
                SDL_Point globalPosition = getGlobalPosition();
                int x = event.button.x - globalPosition.x;
                int y = event.button.y - globalPosition.y;
                // Move cursor to click
                moveCursorTo(x, y);
            }
            break;
        case SDL_MOUSEBUTTONUP:
            break;
        case SDL_KEYDOWN:
            handleKeypress(event.key.keysym);
            break;
        case SDL_KEYUP:
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

std::string TextBox::getText() {
    std::stringstream ss;
    for(size_t i = 0; i < lineTextures.size(); i++) {
        ss << lineTextures[i]->getText();
        if(!lineTextures[i]->wrapped) ss << '\n';
    }
    return ss.str();
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
    // TODO implement word-wrap and undrawn space
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
        if(_lineIndexOfCursor == i && _characterIndexOfCursor > lineTextures[i]->numCharacters()) {
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
        // Check if cursor needs to be moved
        int lengthOfLine = newLine->numCharacters();
        if(_lineIndexOfCursor == lastLineWrapped + newLinesCreated) {
            // The cursor is on this line
            // Check if the cursor should move to the next line
            if(_characterIndexOfCursor > lengthOfLine ||
               _characterIndexOfCursor == lengthOfLine && text.size() > 0
            ) {
                _lineIndexOfCursor++;
                _characterIndexOfCursor -= lengthOfLine;
            }
        }
    }
    // Make sure last created line is recorded as ending in a newline instead
    lineTextures[lastLineWrapped]->wrapped = true;
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
            // Insert the text into the new line
            insertTextAtCursor(newLineText);
            _characterIndexOfCursor = 0;
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
        // Insert the text into the new line
        insertTextAtCursor(newLineText);
        _characterIndexOfCursor = 0;
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
    // Do nothing if there's nothing before the cursor
    if(_lineIndexOfCursor == 0 && _characterIndexOfCursor == 0) return;
    // Decide what to do
    if(_characterIndexOfCursor == 0) {
        // We can assume line index > 0
        // Check if the previous line is wrapped
        if(lineTextures[_lineIndexOfCursor-1]->wrapped) {
            // Remove last character of previous line
            int count = lineTextures[_lineIndexOfCursor-1]->numCharacters();
            lineTextures[_lineIndexOfCursor-1]->removeRange(count-1, 1);
            reWrapFrom(_lineIndexOfCursor-1);
        }
        else {
            // Remove newline from previous line
            lineTextures[_lineIndexOfCursor-1]->wrapped = true;
            reWrapFrom(_lineIndexOfCursor-1);
        }
    }
    else {
        // Remove character before cursor
        _characterIndexOfCursor--;
        lineTextures[_lineIndexOfCursor]->removeRange(_characterIndexOfCursor, 1);
        // Check if this line is wrapped
        if(lineTextures[_lineIndexOfCursor]->wrapped) {
            // Re-wrap in case the flow of text has changed
            if(_lineIndexOfCursor != 0 && lineTextures[_lineIndexOfCursor-1]->wrapped) {
                reWrapFrom(_lineIndexOfCursor-1);
            }
            else {
                reWrapFrom(_lineIndexOfCursor);
            }
        }
    }
    // Just in case it wasn't already set
    redrawRequested = true;
}

void TextBox::reWrapFrom(int lineIndexToStartFrom) {
    // Note: it is assumed that the starting line is wrapped
    // Note: this function will break if more than one line of text is condensed
    // Do nothing if we're already on the last line
    if(lineIndexToStartFrom == lineTextures.size()-1) {
        return;
    }
    // Starting with the first line, pull text from the next line
    int i = lineIndexToStartFrom;
    for(;; i++) {
        // Pull text from next line
        int initialLengthOfThisLine = lineTextures[i]->numCharacters();
        int charactersPulled = lineTextures[i]->pullTextFrom(lineTextures[i+1], width - margin*2);
        redrawRequested = true;
        // Check if cursor position needs to be updated
        if(_lineIndexOfCursor == i+1) {
            // Cursor is on the next line
            _characterIndexOfCursor -= charactersPulled;
            // If cursor has been pulled into this line, calculate that too
            if(_characterIndexOfCursor < 0) {
                _lineIndexOfCursor--;
                _characterIndexOfCursor += lineTextures[_lineIndexOfCursor]->numCharacters();
            }
        }
        // Stop if a newline is encountered
        if(!lineTextures[i+1]->wrapped) break;
        // Stop if there's only one more line left
        if(i == lineTextures.size() - 2) break;
    }
    // At this point, lines[i] is the most recently wrapped line
    // If a line was emptied, remove it and shift following lines up
    if(lineTextures[i+1]->numCharacters() == 0) {
        // "This line": the line before the one that was emptied (lines[i])
        // This line takes the cursor if the next line has it
        if(_lineIndexOfCursor == i+1) {
            _lineIndexOfCursor = i;
            _characterIndexOfCursor = lineTextures[i]->numCharacters();
        }
        // This line adopts line ending of emptied line
        lineTextures[i]->wrapped = lineTextures[i+1]->wrapped;
        // Delete emptied line
        lineTextures.erase(lineTextures.begin()+i+1);
        // Find the location of this line
        SDL_Rect rect = lineTextures[i]->getRect();
        SDL_Point location;
        location.x = rect.x;
        location.y = rect.y;
        // Place following lines in sequence after this one
        for(i++; i < lineTextures.size(); i++) {
            // Step the location one line down
            location.y += fontSize + lineSpacing;
            // Move the line there
            lineTextures[i]->moveLine(location);
        }
        // Resize the texture to fit the number of lines
        rect = getRect();
        resizeTexture(rect.w, rect.h - fontSize - lineSpacing);
    }
}

void TextBox::moveCursorTo(int x, int y) {
    redrawRequested = true;
    // Find out line
    _lineIndexOfCursor = (y - margin) / (fontSize + lineSpacing);
    if(_lineIndexOfCursor < 0) {
        _lineIndexOfCursor = 0;
        _characterIndexOfCursor = 0;
        return;
    }
    if(_lineIndexOfCursor >= lineTextures.size()) {
        _lineIndexOfCursor = lineTextures.size()-1;
        _characterIndexOfCursor = lineTextures[_lineIndexOfCursor]->numCharacters();
        return;
    }
    // Find out character index
    _characterIndexOfCursor = lineTextures[_lineIndexOfCursor]->indexAtXPos(x - margin);
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
