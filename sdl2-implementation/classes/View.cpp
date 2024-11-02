#include "View.h"
#include "TextNode.h"
#include <iostream>

View::View(SDL_Renderer* renderer)
    : Renderable(renderer)
{
    std::string text = "Hello World";
    nodes.push_back(std::shared_ptr<Node>(new TextNode(renderer, text)));
    nodes.push_back(std::shared_ptr<Node>(new TextNode(renderer, text)));
    nodes.push_back(std::shared_ptr<Node>(new TextNode(renderer, text)));
}

void View::handleEvent(const SDL_Event& event) {
    // Determine what type of event it is
    switch(event.type) {
    case SDL_MOUSEMOTION:
        mousePosition.x = event.motion.x;
        mousePosition.y = event.motion.y;
        mouseVelocity.x = event.motion.xrel;
        mouseVelocity.y = event.motion.yrel;
        switch(state) {
        case State::Dragging:
            _nodeThatIsDirectTargetOfDrag->translate(mouseVelocity.x, mouseVelocity.y);
            redrawRequested = true; // TODO implement partial redraw system
            break;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        switch(state) {
        case State::Waiting:
            // TODO make sure the left button is clicked
            auto nodeThatWasClicked = getNodeAtMouse();
            if(nodeThatWasClicked == nullptr) {
                // TODO start selecting
            }
            else {
                switchToStateDragging(nodeThatWasClicked, shiftIsPressed);
            }
            break;
        }
        break;
    case SDL_MOUSEBUTTONUP:
        switch(state) {
        case State::Dragging:
            // TODO make sure the left button is being released
            switchToStateWaiting();
            break;
        }
        break;
    case SDL_WINDOWEVENT:
        break;
    default:
        // Ignore event
        break;
    }
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

void View::switchToStateWaiting() {
    resetState();
    state = State::Waiting;
    SDL_Log("State: Waiting");
}

void View::switchToStateDragging(std::shared_ptr<Node> nodeToBeDragged, bool shiftButtonIsPressed) {
    resetState();
    state = State::Dragging;
    SDL_Log("State: Dragging");
    // If you try to drag a node that's not selected, other nodes will get deselected
    if(!nodeToBeDragged->isSelected) {
        // TODO add isSelected to Node class
        // TODO if(!shiftButtonIsPressed) clearSelection();
    }
    // Otherwise, the user might be trying to interact with the node
    else {
        _userMightBeTryingToInteractWithNode = true;
    }
    // Add current node to the selection, or at least make sure it's selected
    // TODO addNodeToSelection(nodeToBeDragged);
    // Record this node
    _nodeThatIsDirectTargetOfDrag = nodeToBeDragged;
    // Bring this node to the front
    moveNodeToFront(nodeToBeDragged);
}

void View::resetState() {
    switch(state) {
    case State::Waiting:
        /*
        turnOffArrowHandleSystem();
        _nodeThatIsBeingInteractedWith = nullptr;
        */
        break;
    case State::Dragging:
        _nodeThatIsDirectTargetOfDrag = nullptr;
        _userMightBeTryingToInteractWithNode = false;
        break;
    /*
    case State::Selecting:
        //selectionBox.style.visibility = "collapse"; TODO
        _mousePositionAtStartOfSelection = nullptr;
        break;
    case State::Interacting:
        if(isLabelNode(_nodeThatIsBeingInteractedWith) && _nodeThatIsBeingInteractedWith.isEmpty()) { TODO
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
      */
    }
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

bool _arrowHandleSystemIsOn;
Node* _nodeThatHasArrowHandle;
void View::turnOnArrowHandleSystem() {
}
void View::turnOffArrowHandleSystem() {
}
*/

std::shared_ptr<Node> View::getNodeAtMouse() {
    for(int i = nodes.size() - 1; i >= 0; i--) {
        SDL_Rect rect = nodes[i]->getRect();
        if(SDL_PointInRect(&mousePosition, &rect)) {
            return nodes[i];
        }
    }
    return {};
}

void View::moveNodeToFront(std::shared_ptr<Node> node) {
    for(size_t i = 0; i < nodes.size(); i++) {
        if(node == nodes[i]) {
            nodes.erase(nodes.begin() + i);
            nodes.push_back(node);
            return;
        }
    }
    SDL_Log("Error: node being moved to front isn't in node list.");
}
