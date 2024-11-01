#include "View.h"
#include "TextNode.h"
#include <iostream>

View::View(SDL_Renderer* renderer)
    : Renderable(renderer)
{
    std::string text = "Hello World";
    nodes.push_back(std::shared_ptr<Node>(new TextNode(renderer, text)));
}

void View::handleEvent(const SDL_Event& event) {
    // Determine what type of event it is TODO
}

void View::_render(SDL_Renderer* renderer) {
    // Draw background // TODO: only draw background if we actually want to reset everything
    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
    SDL_RenderClear(renderer);
    // Render arrows TODO
    // Render nodes
    for(size_t i = 0; i < nodes.size(); i++) {
        drawChild(*nodes[i]);
    }
    // Reset overlap rects
    overlapRects.clear();
}

/*

void View::switchToStateWaiting() {
}

bool _userMightBeTryingToInteractWithNode;
Node* _nodeThatIsDirectTargetOfDrag;
void View::switchToStateDragging(Node* nodeToBeDragged, bool shiftButtonIsPressed) {
}

Vector2 _mousePositionAtStartOfSelection;
void View::switchToStateSelecting() {
}

Node* _nodeThatIsBeingInteractedWith;
void View::switchToStateInteracting() {
}

//Arrow* _arrowThatIsBeingCreated TODO
Node* _nodeThatArrowMightConnectTo;
void View::switchToStateNewArrow() {
}

//Arrow* _arrowThatIsBeingDragged: Arrow = null; TODO
float _tValueOfArrowBeingDragged;
void View::switchToStateDraggingArrow() {
}

//Arrow* _arrowThatIsBeingInteractedWith TODO
void View::switchToStateInteractingWithArrow() {
}

void View::resetState() {
    switch(state) {
    case State::Waiting:
        turnOffArrowHandleSystem();
        _nodeThatIsBeingInteractedWith = nullptr;
        break;
    case State::Dragging:
        _nodeThatIsDirectTargetOfDrag = nullptr;
        _userMightBeTryingToInteractWithNode = false;
        break;
    case State::Selecting:
        //selectionBox.style.visibility = "collapse"; TODO
        _mousePositionAtStartOfSelection = nullptr;
        break;
    case State::Interacting:
        /*if(isLabelNode(_nodeThatIsBeingInteractedWith) && _nodeThatIsBeingInteractedWith.isEmpty()) { TODO
            deleteNode(_nodeThatIsBeingInteractedWith);
        } <end comment>
        //_nodeThatIsBeingInteractedWith.stopInteraction(); TODO
        _nodeThatIsBeingInteractedWith = nullptr;
        turnOffArrowHandleSystem();
        break;
    case State::NewArrow:
        _arrowThatIsBeingCreated = nullptr;
        _nodeThatArrowMightConnectTo = nullptr;
        break;
    case State::DraggingArrow:
      _arrowThatIsBeingDragged = nullptr;
      _tValueOfArrowBeingDragged = nullptr;
      break;
    case State::InteractingWithArrow:
      turnOffArrowHandleSystem();
      _arrowThatIsBeingInteractedWith.stopInteraction();
      _arrowThatIsBeingInteractedWith = nullptr;
      turnOffArrowHandleSystem();
      break;
    }
}

bool _arrowHandleSystemIsOn;
Node* _nodeThatHasArrowHandle;
void View::turnOnArrowHandleSystem() {
}
void View::turnOffArrowHandleSystem() {
}
*/
