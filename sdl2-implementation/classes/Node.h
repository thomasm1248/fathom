#pragma once

#include <SDL.h>
#include "Renderable.h"

class Node : public Renderable
{
public:
    Node(SDL_Renderer* renderer) : Renderable(renderer) {}
    ~Node() = default;

private:
};
