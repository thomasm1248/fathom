#pragma once

#include <SDL.h>

class Renderable
{
public:
    Renderable(SDL_Renderer* renderer);
    ~Renderable();
    SDL_Rect getRect();
    bool requestsToBeRedrawn();
    void render();
    // No copying
    Renderable(Renderable const& other) = delete;
    Renderable& operator=(Renderable const& other) = delete;

protected:
    SDL_Renderer* renderer;
    bool redrawRequested = true;

    void drawChild(Renderable& child);
    void drawChildIntoClipRect(Renderable& child, SDL_Rect const& rect);
    void initializeTexture(int width, int height);
    void resizeTexture(int width, int height);
    void moveTexture(int x, int y);
    bool hasMoved();
    virtual void _render(SDL_Renderer* renderer) = 0; // implemented by derived class

private:
    SDL_Texture* currentTexture = NULL;
    SDL_Point position;
    bool moved = true;
};