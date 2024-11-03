#pragma once

#include <SDL.h>
#include "Renderable.h"

class Node : public Renderable
{
public:
    bool isSelected = false;

    Node(SDL_Renderer* renderer) : Renderable(renderer) {}
    void translate(int dx, int dy);

private:
};
