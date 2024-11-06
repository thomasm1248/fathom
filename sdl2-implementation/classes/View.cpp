#include "View.h"
#include "TextNode.h"
#include <iostream>

View::View(SDL_Renderer* renderer)
    : Renderable(renderer)
{
    std::string text = "Hello World\nThis is a test.\nAnother line that's actually really long.\nBye.";
    nodes.push_back(std::shared_ptr<Node>(new TextNode(renderer, text)));
    nodes.push_back(std::shared_ptr<Node>(new TextNode(renderer, text)));
    nodes.push_back(std::shared_ptr<Node>(new TextNode(renderer, text)));
}

void View::handleEvent(const SDL_Event& event) {
    // Determine what type of event it is
    switch(event.type) {
    case SDL_KEYDOWN:
        switch(state) {
        case State::Interacting:
            // Pass the event on to the node
            _nodeThatIsBeingInteractedWith->handleEvent(event);
            break;
        }
        break;
    case SDL_KEYUP:
        switch(state) {
        case State::Interacting:
            // Pass the event on to the node
            _nodeThatIsBeingInteractedWith->handleEvent(event);
            break;
        }
        break;
    case SDL_TEXTINPUT:
        switch(state) {
        case State::Interacting:
            // Pass the event on to the node
            _nodeThatIsBeingInteractedWith->handleEvent(event);
            break;
        }
        break;
    case SDL_MOUSEMOTION:
        mousePosition.x = event.motion.x;
        mousePosition.y = event.motion.y;
        mouseVelocity.x = event.motion.xrel;
        mouseVelocity.y = event.motion.yrel;
        // If hovering system is on, updated which node is being hovered
        if(_hoveringSystemIsOn) {
            auto nodeThatIsHoveredNow = getNodeAtMouse();
            if(nodeThatIsHoveredNow != _nodeThatIsBeingHovered) {
                // Unhover the previously hovered node if there is one
                if(_nodeThatIsBeingHovered != nullptr) {
                    _nodeThatIsBeingHovered->isHovered(false);
                    _nodeThatIsBeingHovered = nullptr;
                }
                // Hover newly hovered node if there is one
                if(nodeThatIsHoveredNow != nullptr) {
                    nodeThatIsHoveredNow->isHovered(true);
                    _nodeThatIsBeingHovered = nodeThatIsHoveredNow;
                }
            }
        }
        // Do state-specific stuff
        switch(state) {
        case State::Dragging:
            _userMightBeTryingToInteractWithNode = false;
            // Apply translation to all selected nodes
            for(size_t i = 0; i < selectedNodes.size(); i++) {
                selectedNodes[i]->translate(mouseVelocity.x, mouseVelocity.y);
            }
            redrawRequested = true; // TODO implement partial redraw system
            break;
        case State::Interacting:
            // Pass the event on to the node
            _nodeThatIsBeingInteractedWith->handleEvent(event);
            break;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        // If a node wasn't clicked, clear selected nodes
        if(getNodeAtMouse() == nullptr) clearSelection();
        // Do state-specific things
        switch(state) {
        case State::Waiting:
            if(event.button.button == SDL_BUTTON_LEFT) {
                auto nodeThatWasClicked = getNodeAtMouse();
                if(nodeThatWasClicked == nullptr) {
                    // TODO start selecting
                }
                else {
                    SDL_Keymod modState = SDL_GetModState();
                    switchToStateDragging(nodeThatWasClicked, modState & KMOD_SHIFT);
                }
            }
            break;
        case State::Interacting:
            if(event.button.button == SDL_BUTTON_LEFT) {
                auto nodeThatWasClicked = getNodeAtMouse();
                if(nodeThatWasClicked == nullptr) {
                    // TODO start selecting
                    switchToStateWaiting();
                }
                else if(nodeThatWasClicked == _nodeThatIsBeingInteractedWith) {
                    // We're interacting with this node. Let it handle the event
                    _nodeThatIsBeingInteractedWith->handleEvent(event);
                }
                else {
                    // We clicked on another node, start dragging it
                    SDL_Keymod modState = SDL_GetModState();
                    switchToStateDragging(nodeThatWasClicked, modState & KMOD_SHIFT);
                }
            }
            else if(event.button.button == SDL_BUTTON_RIGHT) {
            }
            break;
        }
        break;
    case SDL_MOUSEBUTTONUP:
        switch(state) {
        case State::Dragging:
            if(event.button.button == SDL_BUTTON_LEFT) {
                if(_userMightBeTryingToInteractWithNode) {
                    switchToStateInteracting(_nodeThatIsDirectTargetOfDrag);
                }
                else {
                    switchToStateWaiting();
                }
            }
            break;
        case State::Interacting:
            // Pass event on to the node
            _nodeThatIsBeingInteractedWith->handleEvent(event);
            break;
        }
        break;
    case SDL_WINDOWEVENT:
        redrawRequested = true;
        // TODO only request redraw when actually needed
        break;
    default:
        // Ignore event
        break;
    }
}

void View::_render(SDL_Renderer* renderer) {
    // First Pass: check what needs to be redrawn

    // Second Pass: draw stuff
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

void View::hoveringSystemOn(bool isIt) {
    _hoveringSystemIsOn = isIt;
    if(_hoveringSystemIsOn) {
        // Clear last hovered node just in case
        if(_nodeThatIsBeingHovered != nullptr) {
            _nodeThatIsBeingHovered->isHovered(false);
            _nodeThatIsBeingHovered = nullptr;
        }
        // Set new hovered node if there is one
        _nodeThatIsBeingHovered = getNodeAtMouse();
        if(_nodeThatIsBeingHovered != nullptr) {
            _nodeThatIsBeingHovered->isHovered(true);
        }
    }
    else {
        // Stop hovering currently hovered node if there is one
        if(_nodeThatIsBeingHovered != nullptr) {
            _nodeThatIsBeingHovered->isHovered(false);
            _nodeThatIsBeingHovered = nullptr;
        }
    }
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
    hoveringSystemOn(false);
    // If you try to drag a node that's not selected, other nodes will get deselected
    if(!nodeToBeDragged->isSelected()) {
        if(!shiftButtonIsPressed) clearSelection();
    }
    // Otherwise, the user might be trying to interact with the node
    else {
        _userMightBeTryingToInteractWithNode = true;
    }
    // Add current node to the selection, or at least make sure it's selected
    addNodeToSelection(nodeToBeDragged);
    // Record this node
    _nodeThatIsDirectTargetOfDrag = nodeToBeDragged;
    // Bring this node to the front
    moveNodeToFront(nodeToBeDragged);
    redrawRequested = true;
}

void View::switchToStateInteracting(std::shared_ptr<Node> nodeThatWasClickedOn) {
    resetState();
    state = State::Interacting;
    SDL_Log("State: Interacting");
    // TODO turnOnArrowHandleSystem();
    // Start interacting with node
    _nodeThatIsBeingInteractedWith = nodeThatWasClickedOn;
    _nodeThatIsBeingInteractedWith->startInteraction();
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
        hoveringSystemOn(true);
        break;
    case State::Interacting:
        /* TODO delete empty label nodes
        if(isLabelNode(_nodeThatIsBeingInteractedWith) && _nodeThatIsBeingInteractedWith.isEmpty()) {
            deleteNode(_nodeThatIsBeingInteractedWith);
        }
        */
        _nodeThatIsBeingInteractedWith->stopInteraction();
        _nodeThatIsBeingInteractedWith = nullptr;
        // TODO turnOffArrowHandleSystem();
        break;
    /*
    case State::Selecting:
        //selectionBox.style.visibility = "collapse"; TODO
        _mousePositionAtStartOfSelection = nullptr;
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

void View::clearSelection() {
    for(size_t i = 0; i < selectedNodes.size(); i++) {
        selectedNodes[i]->isSelected(false);
    }
    selectedNodes.clear();
    // TODO _nodeThatWasMostRecentlySelected = nullptr;
}

void View::addNodeToSelection(std::shared_ptr<Node> node) {
    if(!node->isSelected()) selectedNodes.push_back(node);
    node->isSelected(true);
    // TODO _nodeThatWasMostRecentlySelected = node;
}
