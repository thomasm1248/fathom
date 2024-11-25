#include "Renderable.h"
#include <iostream>

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
    // TODO get actual size of window instead of screen
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    SDL_GetRendererOutputSize(renderer, &rect.w, &rect.h);
    return rect;
}

bool Renderable::requestsToBeRedrawn() {
    return redrawRequested;
}

bool Renderable::hasMoved() {
    return moved;
}

void Renderable::render() {
    SDL_SetRenderTarget(renderer, currentTexture);
    while(redrawRequested) {
        redrawRequested = false;
        _render(renderer);
    }
}

void Renderable::drawChild(Renderable& child) {
    // Set self as the child's parent
    child.parent = this;
    // Make sure child is up-to-date
    child.render();
    // Find out where to draw the texture
    SDL_Rect destination;
    destination.x = child.position.x;
    destination.y = child.position.y;
    SDL_QueryTexture(child.currentTexture, NULL, NULL, &destination.w, &destination.h);
    // Draw the texture
    SDL_SetRenderTarget(renderer, currentTexture);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(renderer, child.currentTexture, NULL, &destination);
    child.moved = false;
}

void Renderable::drawChildIntoClipRect(Renderable& child, SDL_Rect const& clipRect) {
    // Set self as the child's parent
    child.parent = this;
    // Make sure child is up-to-date
    child.render();
    // Get child rect
    SDL_Rect childRect = child.getRect();
    // Find destination rect
    SDL_Rect destination;
    if(!SDL_IntersectRect(&clipRect, &childRect, &destination)) return;
    // Find source rect
    SDL_Rect source;
    source.x = destination.x - childRect.x;
    source.y = destination.y - childRect.y;
    source.w = destination.w;
    source.h = destination.h;
    // Draw from source to destination
    SDL_SetRenderTarget(renderer, currentTexture);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(renderer, child.currentTexture, &source, &destination);
}

void Renderable::resizeTexture(int width, int height) {
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
    SDL_SetTextureBlendMode(currentTexture, SDL_BLENDMODE_BLEND);
    redrawRequested = true;
}

void Renderable::moveTexture(int x, int y) {
    position.x = x;
    position.y = y;
    moved = true;
}

void Renderable::replaceTexture(SDL_Texture* texture) {
    if(currentTexture)
        SDL_DestroyTexture(currentTexture);
    currentTexture = texture;
}

SDL_Point Renderable::getGlobalPosition() {
    SDL_Point globalPosition{0,0};
    Renderable* current = this;
    while(current != nullptr) {
        globalPosition.x += current->position.x;
        globalPosition.y += current->position.y;
        current = current->parent;
    }
    return globalPosition;
}
