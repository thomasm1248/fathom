#include "TextBox.h"
#include <iostream>
#include <sstream>
#include "Util.h"

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
    // Insert text into line
    startEditing();
    insertMultilineTextAtCursor(text);
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
    // Resize node dynamically
    smartResize();
    // Stop displaying cursor
    switchToStateDisplaying();
}

std::string TextBox::getText() {
    std::stringstream ss;
    for(size_t i = 0; i < lineTextures.size(); i++) {
        ss << lineTextures[i]->getText();
        if(!lineTextures[i]->wrapped && i < lineTextures.size()-1) ss << '\n';
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
        // Some lines will have extra space at the end, keep the cursor from going too far to the right
        if(cursorRect.x > rect.w - margin)
            cursorRect.x = rect.w - margin;
        // Draw cursor
        cursorRect.w = 1;
        cursorRect.h = fontSize;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &cursorRect);
    }
}

void TextBox::insertMultilineTextAtCursor(std::string text) {
    // Break text into lines
    auto strings = Util::splitIntoLines(std::move(text));
    // Insert each line into the textbox one-by-one
    for(size_t i = 0; i < strings.size(); i++) {
        if(i != 0) insertNewlineAtCursor();
        insertTextAtCursor(strings[i]);
    }
}

void TextBox::insertTextAtCursor(std::string text) {
    /* Removes the current paragraph, inserts text into it
       and then puts it back.
    */
    // Save the cursor position
    int cursorIndex = cursorAsIndex();
    // Find the first line of the current paragraph
    int firstLineOfParagraph = _lineIndexOfCursor;
    while(firstLineOfParagraph > 0 && lineTextures[firstLineOfParagraph-1]->wrapped)
        firstLineOfParagraph--;
    // Get all the text from the paragraph and insert the new text into it
    _lineIndexOfCursor = firstLineOfParagraph;
    _characterIndexOfCursor = 0;
    int indexAtBeginningOfParagraph = cursorAsIndex();
    int indexOfCursorWithinParagraph = cursorIndex - indexAtBeginningOfParagraph;
    cursorIndex += text.size(); // Move cursor so it's after the inserted text
    text = removeParagraph(firstLineOfParagraph).insert(indexOfCursorWithinParagraph, text);
    // Create new lines for the paragraph
    SDL_Point newParagraphLocation;
    newParagraphLocation.x = margin;
    newParagraphLocation.y = margin + firstLineOfParagraph * (lineSpacing + fontSize);
    auto lines = createParagraphLines(text, newParagraphLocation);
    lineTextures.insert(lineTextures.begin() + firstLineOfParagraph, lines.begin(), lines.end());
    // Update positions of following lines
    SDL_Point updatedLocation = newParagraphLocation;
    updatedLocation.y += lines.size() * (lineSpacing + fontSize);
    int lineAfterParagraph = firstLineOfParagraph + lines.size();
    for(size_t i = lineAfterParagraph; i < lineTextures.size(); i++) {
        // Move line
        lineTextures[i]->moveLine(updatedLocation);
        // Calculate position of next line
        updatedLocation.y += fontSize + lineSpacing;
    }
    // Put the cursor back
    placeCursorAtIndex(cursorIndex);
    // Make sure this textbox gets redrawn
    redrawRequested = true;
    // Resize textbox to accomodate all lines
    int height = margin * 2 + (lineTextures.size()-1) * (fontSize+lineSpacing) + fontSize;
    resizeTexture(width, height);
}

void TextBox::insertNewlineAtCursor() {
    /* Splits the current paragraph in two, inserting a newline
       in-between them.
    */
    int currentLine = _lineIndexOfCursor;
    // Calculate the cursor index
    int cursorIndex = cursorAsIndex() + 1;
    // Split current line
    std::stringstream ss;
    bool currentLineIsWrapped = lineTextures[currentLine]->wrapped;
    ss << lineTextures[currentLine]->insertNewline(_characterIndexOfCursor);
    lineTextures[currentLine]->wrapped = false;
    // Collect the text from the rest of the paragraph
    int firstLineOfParagraph = currentLine + 1;
    if(currentLineIsWrapped) {
        ss << removeParagraph(firstLineOfParagraph);
    }
    // Create lines for the new paragraph
    SDL_Point newParagraphLocation;
    newParagraphLocation.x = margin;
    newParagraphLocation.y = margin + firstLineOfParagraph * (lineSpacing + fontSize);
    auto lines = createParagraphLines(ss.str(), newParagraphLocation);
    lineTextures.insert(lineTextures.begin() + firstLineOfParagraph, lines.begin(), lines.end());
    // Update positions of following lines
    SDL_Point updatedLocation = newParagraphLocation;
    updatedLocation.y += lines.size() * (lineSpacing + fontSize);
    int lineAfterParagraph = firstLineOfParagraph + lines.size();
    for(size_t i = lineAfterParagraph; i < lineTextures.size(); i++) {
        // Move line
        lineTextures[i]->moveLine(updatedLocation);
        // Calculate position of next line
        updatedLocation.y += fontSize + lineSpacing;
    }
    // Place cursor at new cursor index
    placeCursorAtIndex(cursorIndex);
    // Redraw and resize
    redrawRequested = true;
    int height = margin * 2 + (lineTextures.size()-1) * (fontSize+lineSpacing) + fontSize;
    resizeTexture(width, height);
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
        else if(_characterIndexOfCursor == lineTextures[_lineIndexOfCursor]->numCharacters() && lineTextures[_lineIndexOfCursor]->wrapped) {
            _lineIndexOfCursor++;
            _characterIndexOfCursor = 0;
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
                if(lineTextures[_lineIndexOfCursor]->wrapped)
                    _characterIndexOfCursor--;
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
    case SDLK_v:
        if(keysym.mod & KMOD_CTRL) {
            // Paste text into textbox
            std::string clipboardText{SDL_GetClipboardText()};
            insertMultilineTextAtCursor(clipboardText);
        }
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

int TextBox::cursorAsIndex() {
    int index = _characterIndexOfCursor;
    // Add size of each previous line
    for(size_t i = 0; i < _lineIndexOfCursor; i++) {
        index += lineTextures[i]->numCharacters();
        if(!lineTextures[i]->wrapped) index++;
    }
    return index;
}

void TextBox::placeCursorAtIndex(const int _index) {
    int index = _index;
    for(size_t i = 0; i < lineTextures.size(); i++) {
        int length = lineTextures[i]->numCharacters();
        if(index < length || index == length && !lineTextures[i]->wrapped) {
            _lineIndexOfCursor = i;
            _characterIndexOfCursor = index;
            return;
        }
        else {
            index -= length;
            if(!lineTextures[i]->wrapped) index--;
        }
    }
}

std::string TextBox::removeParagraph(int indexOfFirstLine) {
    /* Removes the lineTextures for the paragraph starting at
       indexOfFirstLine, and returns the text contained in
       them. As a side-effect, the cursor is placed where the
       paragraph used to be. Also, the folowing lines are
       left where they're currently at.
    */
    // Get all the text from the paragraph
    std::stringstream ss;
    int currentLine = indexOfFirstLine;
    while(true) {
        ss << lineTextures[currentLine]->getText();
        if(!lineTextures[currentLine++]->wrapped)
            break;
    }
    int lineAfterParagraph = currentLine;
    // Delete the paragraph's original lines
    lineTextures.erase(lineTextures.begin() + indexOfFirstLine, lineTextures.begin() + lineAfterParagraph);
    // Return the text
    return ss.str();
}

std::vector<std::shared_ptr<TextLine>> TextBox::createParagraphLines(std::string text, SDL_Point location) {
    return createParagraphLines(text, location, nullptr, nullptr);
}

std::vector<std::shared_ptr<TextLine>> TextBox::createParagraphLines(std::string text, SDL_Point location, int* visibleWidth, int* desiredWidth) {
    std::vector<std::shared_ptr<TextLine>> linesOfNewParagraph;
    // Create first line
    auto firstLine = std::make_shared<TextLine>(renderer, font, location);
    text = firstLine->insertText(text, 0, width - margin*2, visibleWidth, desiredWidth);
    if(text.size() > 0)
        firstLine->wrapped = true;
    linesOfNewParagraph.push_back(firstLine);
    // Create more lines if necessary
    while(text.size() > 0) {
        // Calculate position of next line
        location.y += fontSize + lineSpacing;
        // Create new line, and give it as much of the extra text as it will take
        auto newLine = std::make_shared<TextLine>(renderer, font, location);
        text = newLine->insertText(text, newLine->numCharacters(), width - margin*2, visibleWidth, desiredWidth);
        // If there's still same text remaining, mark this line as wrapped
        if(text.size() > 0)
            newLine->wrapped = true;
        // Add new line to the list
        linesOfNewParagraph.push_back(newLine);
    }
    // Return lines of paragraph
    return linesOfNewParagraph;
}

void TextBox::smartResize() {
    const int maxWidth = 300;
    // Save cursor position
    int cursorIndex = cursorAsIndex();
    // Get all the text
    auto allText = getText();
    // Split the text into lines
    std::vector<std::string> lines;
    std::string delimiter = "\n";
    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = allText.find(delimiter, prev)) != std::string::npos)
    {
        lines.push_back(allText.substr(prev, pos - prev));
        prev = pos + delimiter.size();
    }
    lines.push_back(allText.substr(prev));
    // Calculate the desired width
    Util::replace_all(allText, "\n", "MMMMMM");
    int length;
    TTF_SizeUTF8(font, allText.c_str(), &length, NULL);
    float area = length * fontSize * 3.0f;
    width = std::sqrt(area);
    // If the desired width is too large, skip the first pass
    bool skipFirstPass = width > maxWidth;
    bool skipSecondPass = false; // change this later
    // Prepare to record statistics about the lines
    int desiredWidth = 0;
    int visibleWidth = 0;
    // Do first pass to try to fit all the text into the estimated width
    if(!skipFirstPass) {
        // Delete existing lines
        lineTextures.clear();
        // Convert the lines into wrapped paragraphs
        SDL_Point newParagraphLocation{margin, margin};
        for(size_t i = 0; i < lines.size(); i++) {
            // Create a paragraph for the line
            auto paragraphLines = createParagraphLines(lines[i], newParagraphLocation, &visibleWidth, &desiredWidth);
            // Add new line textures to list
            lineTextures.insert(lineTextures.end(), paragraphLines.begin(), paragraphLines.end());
            // Update location for next paragraph
            newParagraphLocation.y += paragraphLines.size() * (lineSpacing + fontSize);
        }
        // Check if the text requested more width
        if(desiredWidth < width - margin*2)
            skipSecondPass = true;
        else
            width = desiredWidth + margin*2 + 1; // +1 just to give it some extra room
    }
    // Do second pass to finalize the arrangement of the text (if needed)
    if(!skipSecondPass) {
        // Ensure width is within limit
        if(width > maxWidth)
            width = maxWidth;
        // Delete existing lines
        lineTextures.clear();
        // Reset visible width metric
        visibleWidth = 0;
        // Convert the lines into wrapped paragraphs
        SDL_Point newParagraphLocation{margin, margin};
        for(size_t i = 0; i < lines.size(); i++) {
            // Create a paragraph for the line
            auto paragraphLines = createParagraphLines(lines[i], newParagraphLocation, &visibleWidth, nullptr);
            // Add new line textures to list
            lineTextures.insert(lineTextures.end(), paragraphLines.begin(), paragraphLines.end());
            // Update location for next paragraph
            newParagraphLocation.y += paragraphLines.size() * (lineSpacing + fontSize);
        }
    }
    // Finalize width so it matches the longest line
    width = visibleWidth + margin*2;
    // Redraw and resize
    redrawRequested = true;
    int height = margin * 2 + (lineTextures.size()-1) * (fontSize+lineSpacing) + fontSize;
    resizeTexture(width, height);
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
