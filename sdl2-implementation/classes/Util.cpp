#include "Util.h"
#include <iostream>

void Util::replace_all(
    std::string& s,
    std::string const& toReplace,
    std::string const& replaceWith
) {
    std::string buf;
    std::size_t pos = 0;
    std::size_t prevPos;

    // Reserves rough estimate of final size of string.
    buf.reserve(s.size());

    while (true) {
        prevPos = pos;
        pos = s.find(toReplace, pos);
        if (pos == std::string::npos)
            break;
        buf.append(s, prevPos, pos - prevPos);
        buf += replaceWith;
        pos += toReplace.size();
    }

    buf.append(s, prevPos, s.size() - prevPos);
    s.swap(buf);
}

std::vector<std::string> Util::splitIntoLines(std::string&& text) {
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
    return std::move(strings);
}

void Util::print(SDL_FPoint point) {
    std::cout << "(" << point.x << ", " << point.y << ")\n";
}
