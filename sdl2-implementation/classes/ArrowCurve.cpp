#include "ArrowCurve.h"
#include "Util.h"
#include <iostream>

bool ArrowCurve::indicesInitialized = false;
SDL_Vertex ArrowCurve::vertices[ArrowCurve::samples*2];
int ArrowCurve::indices[(ArrowCurve::samples-1) * 6];

ArrowCurve::ArrowCurve(
    SDL_Renderer* renderer,
    SDL_FPoint source,
    SDL_FPoint control,
    SDL_FPoint target
)
    : Renderable(renderer)
    , curve(source, control, target)
{
    recomputeTextureDimensions();
}

void ArrowCurve::setSource(SDL_FPoint source) {
    curve.source = source;
    recomputeTextureDimensions();
}

void ArrowCurve::setControl(SDL_FPoint control) {
    curve.control = control;
    recomputeTextureDimensions();
}

void ArrowCurve::setTarget(SDL_FPoint target) {
    curve.target = target;
    recomputeTextureDimensions();
}

void ArrowCurve::setTargetBody(std::shared_ptr<Renderable> targetBody) {
    this->targetBody = targetBody;
    redrawRequested = true;
}

SDL_Rect ArrowCurve::getOverlapRect() {
    if(hasOverlapRect) {
        hasOverlapRect = false;
        return overlapRect;
    }
    return {0,0,0,0};
}

SDL_FPoint ArrowCurve::getControlPoint() {
    return curve.control;
}

void ArrowCurve::updateSource(SDL_FPoint newSource) {
    // Record current control point information
    SDL_FPoint beeline = Util::subtract(curve.target, curve.source);
    SDL_FPoint nBeeline = Util::normalize(beeline);
    SDL_FPoint control = Util::subtract(curve.control, curve.source);
    float projection = Util::dot(control, nBeeline);
    SDL_FPoint right{-nBeeline.y, nBeeline.x};
    float lengthOfBeeline = Util::length(beeline);
    float sidewaysness = Util::dot(control, right);
    float forwardsness = 0;
    if(lengthOfBeeline != 0)
        forwardsness = projection / Util::length(beeline);
    // Update curve's source point
    curve.source = newSource;
    // Re-calculate control point
    beeline = Util::subtract(curve.target, curve.source);
    nBeeline = Util::normalize(beeline);
    right.x = -nBeeline.y;
    right.y = nBeeline.x;
    projection = Util::length(beeline) * forwardsness;
    control = Util::add(Util::scale(nBeeline, projection), Util::scale(right, sidewaysness));
    curve.control = Util::add(curve.source, control);
    // Prepare for re-render
    recomputeTextureDimensions();
}

void ArrowCurve::updateTarget(SDL_FPoint newTarget) {
    // TODO handle same target and source better
    // Record current control point information
    SDL_FPoint beeline = Util::subtract(curve.target, curve.source);
    SDL_FPoint nBeeline = Util::normalize(beeline);
    SDL_FPoint control = Util::subtract(curve.control, curve.source);
    float projection = Util::dot(control, nBeeline);
    SDL_FPoint right{-nBeeline.y, nBeeline.x};
    float lengthOfBeeline = Util::length(beeline);
    float sidewaysness = Util::dot(control, right);
    float forwardsness = 0;
    if(lengthOfBeeline != 0)
        forwardsness = projection / Util::length(beeline);
    // Update curve's target point
    curve.target = newTarget;
    // Re-calculate control point
    beeline = Util::subtract(curve.target, curve.source);
    nBeeline = Util::normalize(beeline);
    right.x = -nBeeline.y;
    right.y = nBeeline.x;
    projection = Util::length(beeline) * forwardsness;
    control = Util::add(Util::scale(nBeeline, projection), Util::scale(right, sidewaysness));
    curve.control = Util::add(curve.source, control);
    // Prepare for re-render
    recomputeTextureDimensions();
}

void ArrowCurve::_render(SDL_Renderer* renderer) {
    // Find right and left edges
    Curve right = curve.offset(1.5);
    Curve left = curve.offset(-1.5);
    // Translate curves so they're local to the current rect
    SDL_Rect rect = getRect();
    SDL_FPoint origin;
    origin.x = -rect.x;
    origin.y = -rect.y;
    right.translate(origin);
    left.translate(origin);
    // Compute list of vertices
    SDL_Color color{255, 255, 255, 255};
    for(int i = 0; i < ArrowCurve::samples; i++) {
        float t = i / (float)(ArrowCurve::samples - 1);
        ArrowCurve::vertices[i].position = left.bezier(t);
        ArrowCurve::vertices[i].color = color;
        ArrowCurve::vertices[i + ArrowCurve::samples].position = right.bezier(t);
        ArrowCurve::vertices[i + ArrowCurve::samples].color = color;
    }
    // Make sure indices are initialized
    if(!indicesInitialized) {
        initializeIndices();
        indicesInitialized = true;
    }
    // Draw curve
    SDL_RenderGeometry(renderer, NULL, ArrowCurve::vertices, ArrowCurve::samples*2, ArrowCurve::indices, (ArrowCurve::samples-1)*6);
}

void ArrowCurve::recomputeTextureDimensions() {
    // Make a rect around each point
    SDL_Rect sourceBox;
    sourceBox.x = std::floor(curve.source.x - margin);
    sourceBox.y = std::floor(curve.source.y - margin);
    sourceBox.w = std::ceil(curve.source.x - sourceBox.x + 2*margin);
    sourceBox.h = std::ceil(curve.source.y - sourceBox.y + 2*margin);
    SDL_Rect controlBox;
    controlBox.x = std::floor(curve.control.x - margin);
    controlBox.y = std::floor(curve.control.y - margin);
    controlBox.w = std::ceil(curve.control.x - controlBox.x + 2*margin);
    controlBox.h = std::ceil(curve.control.y - controlBox.y + 2*margin);
    SDL_Rect targetBox;
    targetBox.x = std::floor(curve.target.x - margin);
    targetBox.y = std::floor(curve.target.y - margin);
    targetBox.w = std::ceil(curve.target.x - targetBox.x + 2*margin);
    targetBox.h = std::ceil(curve.target.y - targetBox.y + 2*margin);
    // Find the union of the boxes
    SDL_Rect box;
    SDL_UnionRect(&sourceBox, &targetBox, &box);
    SDL_UnionRect(&controlBox, &box, &box);
    // Resize and move
    if(!hasOverlapRect) {
        overlapRect = getRect();
        hasOverlapRect = true;
    }
    moveTexture(box.x, box.y);
    resizeTexture(box.w, box.h);
}

void ArrowCurve::initializeIndices() {
    // Compute indices for drawing
    for(int i = 0; i < ArrowCurve::samples-1; i++) {
        int j = i*6;
        ArrowCurve::indices[j+0] = i;
        ArrowCurve::indices[j+1] = i+1;
        ArrowCurve::indices[j+2] = i+ArrowCurve::samples;
        ArrowCurve::indices[j+3] = i+1;
        ArrowCurve::indices[j+4] = i+ArrowCurve::samples;
        ArrowCurve::indices[j+5] = i+ArrowCurve::samples+1;
    }
}
