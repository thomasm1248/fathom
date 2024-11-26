#pragma once

#include "Renderable.h"
#include <memory>
#include "Curve.h"

class ArrowCurve : public Renderable
{
public:
    ArrowCurve(
        SDL_Renderer* renderer,
        SDL_FPoint source,
        SDL_FPoint control,
        SDL_FPoint target
    );
    void setSource(SDL_FPoint source);
    void setControl(SDL_FPoint control);
    void setTarget(SDL_FPoint target);
    SDL_FPoint getSource();
    SDL_FPoint getControl();
    SDL_FPoint getTarget();
    void setTargetBody(std::shared_ptr<Renderable> targetBody);
    void removeTargetBody();
    SDL_Rect getOverlapRect();
    SDL_FPoint getControlPoint();
    void updateSource(SDL_FPoint newSource);
    void updateTarget(SDL_FPoint newTarget);

private:
    Curve curve;
    std::shared_ptr<Renderable> targetBody;
    SDL_Rect overlapRect;
    bool hasOverlapRect;
    static constexpr float margin = 10;

    void _render(SDL_Renderer* renderer);
    void recomputeTextureDimensions();



    // Shared drawing util
    static constexpr int samples = 30;
    static SDL_Vertex vertices[];
    static int indices[];
    static bool indicesInitialized;
    static void initializeIndices();
};
