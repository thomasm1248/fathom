#include "TextLine.h"
#include <iostream>

TextLine::TextLine(SDL_Renderer* renderer, TTF_Font* font, const SDL_Point& location)
    : font(font)
    , Renderable(renderer)
{
    moveTexture(location.x, location.y);
}

int TextLine::numCharacters() {
    return text.size();
}

int TextLine::indexAtXPos(int x) {
    // Snap index to lower boundary
    if(x <= 0) return 0;
    // Find the index and position of the character x is in
    int leftBoundOfCharacter;
    int index;
    TTF_MeasureUTF8(font, text.c_str(), x, &leftBoundOfCharacter, &index);
    // Snap index to upper boundary
    if(index == text.size()) return index;
    // Find position of next character
    int rightBoundOfCharacter;
    TTF_MeasureUTF8(font, text.substr(0, index+1).c_str(), std::numeric_limits<int>::max(), &rightBoundOfCharacter, NULL);
    // Calculate midpoint
    int midpoint = (leftBoundOfCharacter + rightBoundOfCharacter) / 2;
    // If x is past midpoint, increment index
    if(x > midpoint) index++;
    // Return the calculated index
    return index;
}

int TextLine::xPosAtIndex(int index) {
    std::string textUpToIndex = text.substr(0, index);
    int width = 0;
    if(TTF_MeasureUTF8(font, textUpToIndex.c_str(), std::numeric_limits<int>::max(), &width, NULL) == -1) {
        SDL_Log("Error: unable to measure text in TextLine::xPosAtIndex");
    }
    return width;
}

std::string TextLine::insertText(std::string newText, int index, int maxWidth) {
    text.insert(index, newText);
    int finalWidth;
    int finalCharacters;
    if(TTF_MeasureUTF8(font, text.c_str(), maxWidth, &finalWidth, &finalCharacters)) {
        SDL_Log("Error: failed to measure text in TextLine::insertText");
        return "";
    }
    std::string leftoverText = text.substr(finalCharacters);
    text = text.substr(0, finalCharacters);
    redrawRequested = true;
    return leftoverText;
}

std::string TextLine::insertNewline(int index) {
    // Get the text that will be moved to the next line
    auto extraText = text.substr(index);
    // Cut off the extra text from this line's text
    text = text.substr(0, index);
    // Since a newline has been inserted, this line ends with a newline
    wrapped = false;
    // Make sure this line is redrawn
    redrawRequested = true;
    // Return the text that will be moved to the next line
    return extraText;
}

void TextLine::moveLine(SDL_Point newLocation) {
    moveTexture(newLocation.x, newLocation.y);
}

void TextLine::removeRange(int startIndex, int count) {
    // Remove range of text from string
    text.erase(startIndex, count);
    redrawRequested = true;
}

int TextLine::pullTextFrom(std::shared_ptr<TextLine> other, int maxWidth) {
    // Note: returns number of characters pulled
    int initialLength = text.size();
    std::string leftoverText = insertText(other->text, text.size(), maxWidth);
    if(leftoverText == other->text) {
        // Nothing happened
        return 0;
    }
    // Give other the leftover text
    other->text = leftoverText;
    other->redrawRequested = true;
    // Calculate number of characters pulled
    return text.size() - initialLength;
}

void TextLine::_render(SDL_Renderer* renderer) {
    // Draw nothing if the string is empty
    if(text.size() == 0) {
        replaceTexture(NULL); // erase texture
        return;
    }
    // Generate surface
    SDL_Color textColor{255, 255, 255, 255};
    auto surface = TTF_RenderUTF8_Blended(font, text.c_str(), textColor);
    if(!surface) {
        SDL_Log("Error: unable to create text surface.");
        return;
    }
    // Generate texture
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, &(*surface));
    if(!texture) {
        SDL_Log("Error: unable to create texture from surface.");
        return;
    }
    replaceTexture(texture);
    texture = NULL;
    // Free resources
    SDL_FreeSurface(surface);
}
