#pragma once

#include <SDL.h>

class Curve
{
public:
    SDL_FPoint source;
    SDL_FPoint control;
    SDL_FPoint target;

    Curve();
    Curve(SDL_FPoint source, SDL_FPoint control, SDL_FPoint target);
    SDL_FPoint bezier(float t);
    Curve offset(float offset);
    void translate(SDL_FPoint shift);
};
