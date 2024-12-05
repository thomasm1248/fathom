#include "Label.h"
#include "Util.h"

Label::Label(SDL_Renderer* renderer, std::shared_ptr<Font> font, SDL_Point location)
    : Renderable(renderer)
    , renderer(renderer)
    , font(font)
    , lineSpacing(font->getSize() * 0.3)
    , fontSize(font->getSize())
    , location(location)
{
    initializeTexture(1, 1);
    SDL_Point lineLocation{0, 0}; // It doesn't matter what this is
    lines.push_back(std::make_shared<TextLine>(renderer, font->getFont(), lineLocation));
    arrange();
}

Label::Label(SDL_Renderer* renderer, std::shared_ptr<Font> font, SDL_Point location, std::string text)
    : Renderable(renderer)
    , renderer(renderer)
    , font(font)
    , lineSpacing(font->getSize() * 0.3)
    , fontSize(font->getSize())
    , location(location)
{
    initializeTexture(1, 1);
    SDL_Point lineLocation{0, 0}; // It doesn't matter what this is
    lines.push_back(std::make_shared<TextLine>(renderer, font->getFont(), lineLocation));
    auto lines = Util::splitIntoLines(std::move(text));
    for(size_t i = 0; i < lines.size(); i++) {
        if(i != 0) insertNewline();
        insertText(lines[i]);
    }
    arrange();
}

void Label::handleEvent(SDL_Event& event) {
    switch(event.type) {
    case SDL_KEYDOWN:
        if(event.key.keysym.sym == SDLK_BACKSPACE) {
            backspace();
        }
        else if(event.key.keysym.sym == SDLK_RETURN) {
            insertNewline();
        }
        break;
    case SDL_TEXTINPUT:
        std::string inputText = std::string(event.text.text);
        insertText(inputText);
        break;
    }
}

void Label::setPosition(SDL_Point newLocation) {
    location = newLocation;
    // Move texture so it's centered over its assigned position
    if(!hasOverlapRect) {
        overlapRect = getRect();
        hasOverlapRect = true;
    }
    SDL_Rect currentRect = getRect();
    moveTexture(location.x - currentRect.w/2, location.y - currentRect.h/2);
}

void Label::reset() {
    // Delete current lines
    lines.clear();
    // Create a new line
    SDL_Point lineLocation{0, 0}; // It doesn't matter what this is
    lines.push_back(std::make_shared<TextLine>(renderer, font->getFont(), lineLocation));
    arrange();
}

bool Label::getOverlapRect(SDL_Rect& rect) {
    if(hasOverlapRect) {
        hasOverlapRect = false;
        rect = overlapRect;
        return true;
    }
    return false;
}

void Label::_render(SDL_Renderer* renderer) {
    // Draw background
    auto backgroundColor = font->getBackground();
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_Rect rect = getRect();
    rect.x = 0;
    rect.y = 0;
    SDL_RenderFillRect(renderer, &rect);
    // Draw lines of text
    for(size_t i = 0; i < lines.size(); i++) {
        drawChild(*lines[i]);
    }
}

void Label::arrange() {
    // Find the widest line
    int maxWidth = 0;
    for(size_t i = 0; i < lines.size(); i++) {
        int sizeOfThisLine = lines[i]->getRect().w;
        if(sizeOfThisLine > maxWidth)
            maxWidth = sizeOfThisLine;
    }
    // Re-position all the lines
    SDL_Point lineLocation{0, margin};
    for(size_t i = 0; i < lines.size(); i++) {
        int sizeOfThisLine = lines[i]->getRect().w;
        lineLocation.x = (maxWidth - sizeOfThisLine) / 2 + margin;
        lines[i]->moveLine(lineLocation);
        lineLocation.y += margin + lineSpacing + fontSize;
    }
    // Resize texture
    if(!hasOverlapRect) {
        overlapRect = getRect();
        hasOverlapRect = true;
    }
    resizeTexture(maxWidth + margin*2, lines.size() * (lineSpacing + fontSize) - lineSpacing + margin*2);
    // Move texture so it's centered over its assigned position
    SDL_Rect currentRect = getRect();
    moveTexture(location.x - currentRect.w/2, location.y - currentRect.h/2);
}

void Label::insertText(std::string text) {
    auto line = lines[lines.size()-1]; // Get the last line
    line->insertText(text, line->numCharacters(), 2000000000);
    arrange();
}

void Label::insertNewline() {
    SDL_Point lineLocation{0, 0}; // It doesn't matter what this is
    lines.push_back(std::make_shared<TextLine>(renderer, font->getFont(), lineLocation));
    arrange();
}

void Label::backspace() {
    auto line = lines[lines.size()-1]; // Get last line
    if(line->numCharacters() == 0) {
        // Remove last line
        lines.erase(lines.end() - 1);
    }
    else {
        // Remove last character of last line
        std::string text = line->getText();
        text = text.substr(0, text.size()-1); // Remove last character
        SDL_Point lineLocation{0, 0}; // It doesn't matter what this is
        line = std::make_shared<TextLine>(renderer, font->getFont(), lineLocation);
        line->insertText(text, 0, 2000000000);
        lines[lines.size()-1] = line;
    }
    arrange();
}
