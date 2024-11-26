#include "ArrowHandle.h"

ArrowHandle::ArrowHandle(SDL_Renderer* renderer)
    : Renderable(renderer)
{
    initializeTexture(1, 1);
}

void ArrowHandle::setToNode(std::shared_ptr<Node> nodeWithArrowHandle) {
    if(active && !hasOverlapRect) {
        overlapRect = getRect();
        hasOverlapRect = true;
    }
    if(!nodeWithArrowHandle) {
        reset();
        return;
    }
    SDL_Rect rect = nodeWithArrowHandle->getRect();
    int handleWidth = 20;
    rect.x -= handleWidth;
    rect.y -= handleWidth;
    rect.w += handleWidth*2;
    rect.h += handleWidth*2;
    moveTexture(rect.x, rect.y);
    resizeTexture(rect.w, rect.h);
    active = true;
}

void ArrowHandle::reset() {
    if(!active) return;
    if(!hasOverlapRect) {
        overlapRect = getRect();
        hasOverlapRect = true;
    }
    active = false;
}

bool ArrowHandle::getOverlapRect(SDL_Rect& overlapRect) {
    if(hasOverlapRect) {
        hasOverlapRect = false;
        overlapRect = this->overlapRect;
        return true;
    }
    else {
        return false;
    }
}
    
bool ArrowHandle::isActive() {
    return active;
}

void ArrowHandle::_render(SDL_Renderer* renderer) {
    SDL_Rect rect = getRect();
    rect.x = 0;
    rect.y = 0;
    SDL_SetRenderDrawColor(renderer, 14, 150, 158, 255);
    SDL_RenderFillRect(renderer, &rect);
}
