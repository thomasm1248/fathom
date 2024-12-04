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

SDL_FPoint ArrowCurve::getSource() {
    return curve.source;
}

SDL_FPoint ArrowCurve::getControl() {
    return curve.control;
}

SDL_FPoint ArrowCurve::getTarget() {
    return curve.target;
}

void ArrowCurve::setTargetBody(std::shared_ptr<Renderable> targetBody) {
    this->targetBody = targetBody;
    redrawRequested = true;
}

void ArrowCurve::removeTargetBody() {
    targetBody = nullptr;
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
    SDL_Color color{255, 255, 255, 255};
    // Find where the curve should stop
    float endOfCurve = getEndOfCurve();
    // Compute direction of arrowhead
    auto locationOfArrowhead = curve.bezier(endOfCurve);
    auto justBeforeArrowhead = curve.bezier(endOfCurve - 0.01);
    auto vector = Util::subtract(locationOfArrowhead, justBeforeArrowhead);
    float direction = std::atan2(vector.y, vector.x);
    // Draw arrowhead
    auto textureLocation = getRect();
    locationOfArrowhead.x -= textureLocation.x;
    locationOfArrowhead.y -= textureLocation.y;
    float point = 10;
    float side = 7;
    float sideRotation = 2;
    SDL_Vertex arrowhead[3];
    arrowhead[0].position.x = locationOfArrowhead.x + std::cos(direction) * point;
    arrowhead[0].position.y = locationOfArrowhead.y + std::sin(direction) * point;
    arrowhead[0].color = color;
    arrowhead[1].position.x = locationOfArrowhead.x + std::cos(direction + sideRotation) * side;
    arrowhead[1].position.y = locationOfArrowhead.y + std::sin(direction + sideRotation) * side;
    arrowhead[1].color = color;
    arrowhead[2].position.x = locationOfArrowhead.x + std::cos(direction - sideRotation) * side;
    arrowhead[2].position.y = locationOfArrowhead.y + std::sin(direction - sideRotation) * side;
    arrowhead[2].color = color;
    SDL_RenderGeometry(renderer, NULL, arrowhead, 3, NULL, 0);
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
    for(int i = 0; i < ArrowCurve::samples; i++) {
        float t = i / (float)(ArrowCurve::samples - 1) * endOfCurve;
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

float ArrowCurve::getEndOfCurve() {
    // Draw the full length of the curve if there's no target body
    if(!targetBody) return 1;
    // If there is a target body, cut the curve short
    // Calculate the box that the curve must not enter
    SDL_Rect targetRect = targetBody->getRect();
    targetRect.x -= targetMargin;
    targetRect.y -= targetMargin;
    targetRect.w += targetMargin*2;
    targetRect.h += targetMargin*2;
    // Search for the t value of where the curve enters the box
    float t = 0.5;
    float increment = 0.25;
    for(int i = 0; i < 20; i++) {
        SDL_FPoint pointOnCurve = curve.bezier(t);
        if(
            pointOnCurve.x < targetRect.x ||
            pointOnCurve.y < targetRect.y ||
            pointOnCurve.x > targetRect.x + targetRect.w ||
            pointOnCurve.y > targetRect.y + targetRect.h
        ) {
            // The point is outside of the box
            t += increment;
        }
        else {
            // The point is inside the box
            t -= increment;
        }
        increment *= .5;
    }
    return t;
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
