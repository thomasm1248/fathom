#include "TextBox.h"
#include <iostream>

TextBox::TextBox(std::string text)
{
    // Break text into lines
    int lastBreak = 0;
    for(int i = 0; i < text.size(); i++) {
        if(text.at(i) == '\n') {
            // Newline found
            lines.push_back(text.substr(lastBreak, i-lastBreak));
            lastBreak = i + 1;
        }
    }
    lines.push_back(text.substr(lastBreak, text.size()-lastBreak));
}
