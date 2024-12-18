#include "TextLine.h"
#include <iostream>
#include <sstream>

TextLine::TextLine(SDL_Renderer* renderer, TTF_Font* font, const SDL_Point& location)
    : font(font)
    , Renderable(renderer)
{
    initializeTexture(1, 1); // placeholder
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
    return insertText(newText, index, maxWidth, nullptr, nullptr);
}

std::string TextLine::insertText(std::string newText, int index, int maxWidth, int* oVisibleWidth, int* oDesiredWidth) {
    text.insert(index, newText);
    int finalWidth;
    int finalCharacters;
    if(TTF_MeasureUTF8(font, text.c_str(), maxWidth, &finalWidth, &finalCharacters)) {
        SDL_Log("Error: failed to measure text in TextLine::insertText");
        return "";
    }
    // TODO "Texture not created with SDL_TEXTUREACCESS_TARGET" being caught here
    // If we're splitting a word, push the whole word off to the next line
    bool wordWasTooBig = false; // flag used later to calculate oDesiredWidth
    if( finalCharacters > 0 &&
        finalCharacters < text.size() &&
        text[finalCharacters-1] != ' ' &&
        text[finalCharacters] != ' '
    ) {
        int initialIndex = finalCharacters; // take note in case we need to reset it
        while(text[finalCharacters-1] != ' ') {
            finalCharacters--;
            if(finalCharacters == 0) {
                // The entire line consisted of one word
                // Don't wrap after all
                finalCharacters = initialIndex;
                wordWasTooBig = true;
                break;
            }
        }
    }
    // Take as much space at the end of the line as possible
    while(text[finalCharacters] == ' ' && finalCharacters < text.size())
        finalCharacters++;
    // Split the string
    std::string leftoverText = text.substr(finalCharacters);
    text = text.substr(0, finalCharacters);
    // If requested, measure the visible width
    if(oVisibleWidth != nullptr) {
        // Find the index that comes after the last visible character
        int visibleCharacters = finalCharacters;
        while(visibleCharacters > 0 && text[visibleCharacters-1] == ' ')
            visibleCharacters--;
        // Measure the width
        int visibleWidth;
        TTF_SizeUTF8(font, text.substr(0, visibleCharacters).c_str(), &visibleWidth, NULL);
        if(visibleWidth > *oVisibleWidth)
            *oVisibleWidth = visibleWidth;
    }
    // If requested, measure the desired width
    if(oDesiredWidth != nullptr) {
        // If the line contained a single word that was too big to
        // fit on the line, find its length
        if(wordWasTooBig) {
            // Get the first part of the word from this line
            std::stringstream ss;
            ss << text;
            // Get the rest of the word from the leftoverText
            for(size_t i = 0; i < leftoverText.size(); i++)
                if(leftoverText[i] != ' ')
                    ss << leftoverText[i];
            // Measure the word's length
            int desiredWidth;
            TTF_SizeUTF8(font, ss.str().c_str(), &desiredWidth, NULL);
            if(desiredWidth > *oDesiredWidth)
                *oDesiredWidth = desiredWidth;
        }
        // Otherwise, just use the maxWidth
        else {
            if(maxWidth > *oDesiredWidth)
                *oDesiredWidth = maxWidth;
        }
    }
    // Calculate new dimensions
    fitToContent();
    // Return the text that didn't fit on this line
    return leftoverText;
}

std::string TextLine::insertNewline(int index) {
    // Get the text that will be moved to the next line
    auto extraText = text.substr(index);
    // Cut off the extra text from this line's text
    text = text.substr(0, index);
    // Since a newline has been inserted, this line ends with a newline
    wrapped = false;
    // Calculate new dimensions
    fitToContent();
    // Return the text that will be moved to the next line
    return extraText;
}

void TextLine::moveLine(SDL_Point newLocation) {
    moveTexture(newLocation.x, newLocation.y);
}

void TextLine::removeRange(int startIndex, int count) {
    // Remove range of text from string
    text.erase(startIndex, count);
    fitToContent();
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
    // Calculate new dimensions of other line
    other->fitToContent();
    // Calculate number of characters pulled
    return text.size() - initialLength;
}

std::string TextLine::getText() {
    return text;
}

void TextLine::_render(SDL_Renderer* renderer) {
    // Draw nothing if the string is empty
    if(text.size() == 0) {
        replaceTexture(NULL); // erase texture
        return;
    }
    // Generate surface
    SDL_Color textColor{255, 255, 255, 255};
    SDL_Color backgroundColor{0,0,0,255};
    auto surface = TTF_RenderUTF8_Shaded(font, text.c_str(), textColor, backgroundColor);
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

void TextLine::fitToContent() {
    // Calculate new dimensions
    int measuredWidth = 1;
    if(text.size() > 0) {
        TTF_SizeUTF8(font, text.c_str(), &measuredWidth, NULL);
    }
    auto rect = getRect();
    resizeTexture(measuredWidth, rect.h); // TODO use height from Font
}
    
