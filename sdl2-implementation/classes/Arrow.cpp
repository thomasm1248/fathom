#include "Arrow.h"
#include "Util.h"

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
}

void Arrow::updateEndFromMousePosition(SDL_Point mousePosition) {
    if(state != State::Creating) {
        SDL_Log("Error: Arrow::updateEndFromMousePosition() called during the wrong state");
        return;
    }
    // Update our record of the mouse position TODO we probably don't actually need to record this information
    _mousePosition = mousePosition;
    // Record the current target point for later
    SDL_FPoint oldTarget = arrowCurve->getTarget();
    // Move the target point to the mouse position
    SDL_FPoint newTarget;
    newTarget.x = _mousePosition.x;
    newTarget.y = _mousePosition.y;
    arrowCurve->setTarget(newTarget);
    // Calculate additional force to apply to the control point
    float amountTargetWasPulledAwayFromControl = 
        Util::dot(
            Util::normalize(
                Util::subtract(oldTarget, arrowCurve->getControl())
            ),
            Util::subtract(newTarget, oldTarget)
        );
    _controlVelocity =
        Util::add(
            _controlVelocity,
            Util::scale(
                Util::normalize(
                    Util::subtract(
                        arrowCurve->getControl(),
                        arrowCurve->getSource()
                    )
                ),
                amountTargetWasPulledAwayFromControl * 0.03
            )
        );
}

void Arrow::attachToNode(std::shared_ptr<ArrowTerminal> targetNode) {
    this->targetNode = targetNode;
    arrowCurve->setTarget(targetNode->getCenter());
    arrowCurve->setTargetBody(targetNode);
}

void Arrow::disconnectFromTarget() {
    targetNode = nullptr;
    arrowCurve->removeTargetBody();
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

void Arrow::doPhysics() {
    SDL_FPoint midpoint = Util::scale(Util::add(arrowCurve->getSource(), arrowCurve->getTarget()), 0.5);
    SDL_FPoint currentControl = arrowCurve->getControl();
    SDL_FPoint force = Util::scale(Util::subtract(midpoint, currentControl), 0.001);
    _controlVelocity = Util::scale(_controlVelocity, 0.98);
    _controlVelocity = Util::add(_controlVelocity, force);
    arrowCurve->setControl(Util::add(currentControl, _controlVelocity));
}

void Arrow::finalizeCreation() {
    if(state != State::Creating) {
        SDL_Log("Error: Arrow::finalizeCreation() called outside of Creating state");
        return;
    }
    switchToStateNothing();
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
    _controlVelocity.x = 0;
    _controlVelocity.y = 0;
}

void Arrow::switchToStateNothing() {
    resetState();
    state == State::Nothing;
}
