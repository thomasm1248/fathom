#include "Arrow.h"

Arrow::Arrow(SDL_Renderer* renderer, std::shared_ptr<ArrowTerminal> sourceNode, SDL_Point mousePosition)
    : sourceNode(sourceNode)
{
    switchToStateCreating(mousePosition);
    // Initialize curve
    SDL_FPoint start = sourceNode->getCenter();
    SDL_FPoint end;
    end.x = (float)mousePosition.x;
    end.y = (float)mousePosition.y;
    SDL_FPoint control;
    control.x = (start.x + end.x) / 2;
    control.y = (start.y + end.y) / 2;
    arrowCurve = std::shared_ptr<ArrowCurve>(new ArrowCurve(renderer, start, control, end));
    // TODO owner should add this arrow to the node's list
}

Arrow::Arrow(SDL_Renderer* renderer, std::shared_ptr<ArrowTerminal> sourceNode, std::shared_ptr<ArrowTerminal> targetNode, std::string label, SDL_FPoint controlPoint)
    : sourceNode(sourceNode)
    , targetNode(targetNode)
    , label(label)
    , controlPoint(controlPoint)
{
    // Initialize curve
    SDL_FPoint start = sourceNode->getCenter();
    SDL_FPoint end = targetNode->getCenter();;
    arrowCurve = std::shared_ptr<ArrowCurve>(new ArrowCurve(renderer, start, controlPoint, end));
    // TODO owner should add this arrow to the node's list
}

void Arrow::updateEndFromMousePosition(SDL_Point mousePosition) {
    // TODO
}

void Arrow::attachToNode(std::shared_ptr<ArrowTerminal> targetNode) {
    this->targetNode = targetNode;
    arrowCurve->setTargetBody(targetNode);
}

void Arrow::updateSource() {
    arrowCurve->updateSource(sourceNode->getCenter());
}

void Arrow::updateTarget() {
    arrowCurve->updateTarget(targetNode->getCenter());
}

std::string Arrow::getLabel() {
    return label;
}

std::shared_ptr<ArrowTerminal> Arrow::getSourceNode() {
    return sourceNode;
}

std::shared_ptr<ArrowTerminal> Arrow::getTargetNode() {
    return targetNode;
}

void Arrow::resetState() {
    switch(state) {
        case State::Creating:
            break;
        case State::Nothing:
            break;
        case State::Selected:
            break;
        case State::Interacting:
            break;
    }
}

void Arrow::switchToStateCreating(SDL_Point mousePosition) {
    resetState();
    state = State::Creating;
    _mousePosition = mousePosition;
}
