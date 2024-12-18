#pragma once

#include <SDL.h>

class Renderable
{
public:
    Renderable(SDL_Renderer* renderer);
    virtual ~Renderable();
    SDL_Rect getRect();
    bool requestsToBeRedrawn();
    bool hasMoved();
    void render();
    // No copying
    Renderable(Renderable const& other) = delete;
    Renderable& operator=(Renderable const& other) = delete;

protected:
    bool redrawRequested = true;

    void drawChild(Renderable& child);
    void drawChildIntoClipRect(Renderable& child, SDL_Rect const& rect);
    void initializeTexture(int width, int height);
    void resizeTexture(int width, int height);
    void moveTexture(int x, int y);
    void replaceTexture(SDL_Texture* texture);
    virtual void _render(SDL_Renderer* renderer) = 0; // implemented by derived class
    SDL_Point getGlobalPosition();

private:
    SDL_Renderer* renderer;
    SDL_Texture* currentTexture = NULL;
    SDL_Point position;
    Renderable* parent = nullptr;
    bool moved = true;
};
