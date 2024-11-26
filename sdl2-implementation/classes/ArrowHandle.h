#pragma once

#include "Renderable.h"
#include <memory>
#include "Node.h"

class ArrowHandle : public Renderable
{
public:
    ArrowHandle(SDL_Renderer* renderer);
    void setToNode(std::shared_ptr<Node> nodeWithArrowHandle);
    void reset();
    bool getOverlapRect(SDL_Rect& overlapRect);
    bool isActive();

private:
    void _render(SDL_Renderer* renderer);
    bool hasOverlapRect = false;
    SDL_Rect overlapRect;
    bool active = false;
};
