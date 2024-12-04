#pragma once

#include "Renderable.h"
#include <vector>
#include <memory>
#include "Node.h"

class SelectionBox : public Renderable
{
public:
    SelectionBox(SDL_Renderer* renderer);
    bool visible();
    bool getOverlapRect(SDL_Rect& oRect);
    void beginSelection(SDL_Point start);
    void continueSelection(SDL_Point end);
    std::vector<std::shared_ptr<Node>> finishSelection(std::vector<std::shared_ptr<Node>> nodes);

private:
    bool _visible = false;
    SDL_Point start;
    SDL_Point end;
    bool hasOverlapRect = false;
    SDL_Rect overlapRect;

    void _render(SDL_Renderer* renderer);
    void recalculateDimensions();
};
