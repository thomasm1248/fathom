#pragma once

#include <string>
#include <SDL.h>

namespace Util {

void replace_all(
    std::string& s,
    std::string const& toReplace,
    std::string const& replaceWith
);

inline float lerp(float a, float b, float t);
inline float dot(const SDL_FPoint& a, const SDL_FPoint& b);
inline float length(const SDL_FPoint& p);
inline SDL_FPoint subtract(const SDL_FPoint& a, const SDL_FPoint& b);
inline SDL_FPoint add(const SDL_FPoint& a, const SDL_FPoint& b);
inline SDL_FPoint normalize(const SDL_FPoint& p);
inline SDL_FPoint scale(const SDL_FPoint& p, float s);

void print(SDL_FPoint point);

}




// Inline definitions

inline float Util::lerp(float a, float b, float t) {
    return a + t * (b - a);
}

inline float Util::dot(const SDL_FPoint& a, const SDL_FPoint& b) {
    return a.x * b.x + a.y * b.y;
}

inline float Util::length(const SDL_FPoint& p) {
    return std::sqrt(p.x * p.x + p.y * p.y);
}

inline SDL_FPoint Util::subtract(const SDL_FPoint& a, const SDL_FPoint& b) {
    SDL_FPoint p;
    p.x = a.x - b.x;
    p.y = a.y - b.y;
    return p;
}

inline SDL_FPoint Util::add(const SDL_FPoint& a, const SDL_FPoint& b) {
    SDL_FPoint p;
    p.x = a.x + b.x;
    p.y = a.y + b.y;
    return p;
}

inline SDL_FPoint Util::normalize(const SDL_FPoint& p) {
    float l = length(p);
    if(l == 0) return {0,0};
    SDL_FPoint o;
    o.x = p.x / l;
    o.y = p.y / l;
    return o;
}

inline SDL_FPoint Util::scale(const SDL_FPoint& p, float s) {
    SDL_FPoint o;
    o.x = p.x * s;
    o.y = p.y * s;
    return o;
}
