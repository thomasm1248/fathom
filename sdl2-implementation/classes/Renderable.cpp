#include "Renderable.h"


Renderable::Renderable(SDL_Renderer* renderer)
    : renderer(renderer)
{
    position.x = 0;
    position.y = 0;
}

Renderable::~Renderable() {
    SDL_DestroyTexture(currentTexture);
    currentTexture = NULL;
}

SDL_Rect Renderable::getRect() {
    if(currentTexture) {
        SDL_Rect rect;
        rect.x = position.x;
        rect.y = position.y;
        SDL_QueryTexture(currentTexture, NULL, NULL, &rect.w, &rect.h);
        return rect;
    }
    // NULL texture: render to screen directly
    SDL_Rect rect;
    SDL_GetRendererOutputSize(renderer, &rect.w, &rect.h);
    return rect;
}

bool Renderable::requestsToBeRedrawn() {
    return redrawRequested;
}

void Renderable::render() {
    SDL_SetRenderTarget(renderer, currentTexture);
    while(redrawRequested) {
        redrawRequested = false;
        _render(renderer);
    }
}

void Renderable::drawChild(Renderable& child) {
    // Make sure child is up-to-date
    child.render();
    // Find out where to draw the texture
    SDL_Rect destination;
    destination.x = child.position.x;
    destination.y = child.position.y;
    SDL_QueryTexture(child.currentTexture, NULL, NULL, &destination.w, &destination.h);
    // Draw the texture
    SDL_SetRenderTarget(renderer, currentTexture);
    SDL_RenderCopy(renderer, child.currentTexture, NULL, &destination);
    child.moved = false;
}

void Renderable::drawChildIntoClipRect(Renderable& child, SDL_Rect const& clipRect) {
    // Make sure child is up-to-date
    child.render();
    // Get child rect
    SDL_Rect childRect = child.getRect();
    // Find destination rect
    SDL_Rect destination;
    if(childRect.x < clipRect.x) {
        destination.x = clipRect.x;
        destination.w = childRect.x + childRect.w - destination.x;
        if(destination.w > clipRect.w) {
            destination.w = clipRect.w;
        }
        else if(destination.w < 0) return;
    }
    else if(childRect.x >= clipRect.x + clipRect.w) return;
    else {
        destination.x = childRect.x;
        destination.w = childRect.w;
        if(destination.x + destination.w > clipRect.x + clipRect.w) {
            destination.w = clipRect.w - (destination.x - clipRect.x);
        }
    }
    if(childRect.y < clipRect.y) {
        destination.y = clipRect.y;
        destination.h = childRect.y + childRect.h - destination.y;
        if(destination.h > clipRect.h) {
            destination.h = clipRect.h;
        }
        else if(destination.h < 0) return;
    }
    else if(childRect.y >= clipRect.y + clipRect.h) return;
    else {
        destination.y = childRect.y;
        destination.h = childRect.h;
        if(destination.y + destination.h > clipRect.y + clipRect.h) {
            destination.h = clipRect.h - (destination.y - clipRect.y);
        }
    }
    // Find source rect
    SDL_Rect source;
    source.x = destination.x - childRect.x;
    source.y = destination.y - childRect.y;
    source.w = destination.w;
    source.h = destination.h;
    // Draw from source to destination
    SDL_SetRenderTarget(renderer, currentTexture);
    SDL_RenderCopy(renderer, child.currentTexture, &source, &destination);
    child.moved = false;
}

void Renderable::resizeTexture(int width, int height) {
    if(currentTexture)
        initializeTexture(width, height);
}

void Renderable::initializeTexture(int width, int height) {
    if(currentTexture)
        SDL_DestroyTexture(currentTexture);
    currentTexture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        width, height
    );
}

void Renderable::moveTexture(int x, int y) {
    if(currentTexture) {
        position.x = x;
        position.y = y;
        moved = true;
    }
}

void Renderable::replaceTexture(SDL_Texture* texture) {
    if(currentTexture)
        SDL_DestroyTexture(currentTexture);
    currentTexture = texture;
}

bool Renderable::hasMoved() {
    return moved;
}
