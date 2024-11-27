#include "View.h"
#include "TextNode.h"
#include <iostream>
#include "ViewFileJSON.h"
#include <cmath>
#include "Util.h"

View::View(SDL_Renderer* renderer, SDL_Window* window)
    : Renderable(renderer)
    , window(window)
    , renderer(renderer)
    , viewFile(new ViewFileJSON("test.fathom", renderer))
{
    if(viewFile->read(nodes, arrows)) {
        SDL_Log("Read file successfully");
    }
    else {
        SDL_Log("Failed to read file");
        viewFile = nullptr;
    }
    // Initialize arrow handle system
    _arrowHandle = std::shared_ptr<ArrowHandle>(new ArrowHandle(renderer));
}

void View::processDynamicContent() {
    // While creating an arrow, animate it
    if(state == State::NewArrow) {
        _arrowThatIsBeingCreated->doPhysics();
    }
}

void View::checkWhatNeedsToBeRedrawn() {
    overlapRects.clear();
    // Check if the arrow handle has an overlap rect
    SDL_Rect rect;
    if(_arrowHandle->getOverlapRect(rect)) {
        redrawRequested = true;
        overlapRects.push_back(rect);
    }
    // Check all arrows to see if anything needs to be redrawn
    for(size_t i = 0; i < arrows.size(); i++) {
        // Check if the arrow wants stuff underneath it to be redrawn
        SDL_Rect rect = arrows[i]->arrowCurve->getOverlapRect();
        if(!SDL_RectEmpty(&rect)) {
            redrawRequested = true;
            overlapRects.push_back(rect);
        }
        // Check if the node itself requests to be redrawn
        if(!redrawRequested && (arrows[i]->arrowCurve->requestsToBeRedrawn() || arrows[i]->arrowCurve->hasMoved())) {
            redrawRequested = true;
        }
    }
    // Check all nodes to see if anything needs to be redrawn
    for(size_t i = 0; i < nodes.size(); i++) {
        // Check if the node wants stuff underneath it to be redrawn
        SDL_Rect rect = nodes[i]->getOverlapRect();
        if(!SDL_RectEmpty(&rect)) {
            redrawRequested = true;
            overlapRects.push_back(rect);
        }
        // Check if the node itself requests to be redrawn
        if(!redrawRequested && (nodes[i]->requestsToBeRedrawn() || nodes[i]->hasMoved())) {
            redrawRequested = true;
        }
    }
}

void View::handleEvent(const SDL_Event& event) {
    std::string inputText;
    SDL_Keymod modState;
    // Determine what type of event it is
    switch(event.type) {
    case SDL_KEYDOWN:
        // Check for app-wide keyboard shortcuts
        if(event.key.keysym.sym == SDLK_s && event.key.keysym.mod & KMOD_CTRL) {
            viewFile->write(nodes, arrows);
        }
        // Do state-specific behaviors
        switch(state) {
        case State::Waiting:
            if(event.key.keysym.sym == SDLK_BACKSPACE) {
                deleteSelectedNodes();
            }
            break;
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
        case State::Waiting:
            auto newNode = std::shared_ptr<Node>(new TextNode(renderer, mousePosition));
            nodes.push_back(newNode);
            switchToStateInteracting(newNode);
            newNode->handleEvent(event);
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
        // If arrow handle system is on, update which node has arrow handle
        if(_arrowHandleSystemIsOn) {
            auto arrowHandleRect = _arrowHandle->getRect();
            if(_arrowHandle->isActive()) {
                auto nodeUnderMouse = getNodeAtMouse();
                if(!SDL_PointInRect(&mousePosition, &arrowHandleRect) || nodeUnderMouse && nodeUnderMouse != _nodeThatHasArrowHandle) {
                    _nodeThatHasArrowHandle = nodeUnderMouse;
                    _arrowHandle->setToNode(_nodeThatHasArrowHandle);
                }
            }
            else {
                _nodeThatHasArrowHandle = getNodeAtMouse();
                if(_nodeThatHasArrowHandle) {
                    _arrowHandle->setToNode(_nodeThatHasArrowHandle);
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
            break;
        case State::Interacting:
            // Pass the event on to the node
            _nodeThatIsBeingInteractedWith->handleEvent(event);
            break;
        case State::NewArrow:
            {
                auto nodeUnderMouse = getNodeAtMouse();
                if(_nodeThatArrowMightConnectTo != nodeUnderMouse) {
                    // Disconnect arrow from previous node
                    if(_nodeThatArrowMightConnectTo) {
                        _nodeThatArrowMightConnectTo->removeArrow(_arrowThatIsBeingCreated);
                        _arrowThatIsBeingCreated->disconnectFromTarget();
                        _nodeThatArrowMightConnectTo = nullptr;
                    }
                    // Connect arrow to new node
                    if(nodeUnderMouse) {
                        _nodeThatArrowMightConnectTo = nodeUnderMouse;
                        _nodeThatArrowMightConnectTo->addIncomingArrow(_arrowThatIsBeingCreated);
                        _arrowThatIsBeingCreated->attachToNode(_nodeThatArrowMightConnectTo);
                    }
                }
                // Make end of arrow track mouse if it isn't connected to a node
                if(!_nodeThatArrowMightConnectTo) {
                    _arrowThatIsBeingCreated->updateEndFromMousePosition(mousePosition);
                }
            }
            break;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        // If a node wasn't clicked, clear selected nodes
        if(getNodeAtMouse() == nullptr) clearSelection();
        // If arrow handle was clicked, start making an arrow
        if(event.button.button == SDL_BUTTON_LEFT && _arrowHandleSystemIsOn && _arrowHandle->isActive() && getNodeAtMouse() == nullptr) {
            // Check if user clicked on arrow handle
            auto rect = _arrowHandle->getRect();
            if(SDL_PointInRect(&mousePosition, &rect)) {
                switchToStateNewArrow(_nodeThatHasArrowHandle);
                break; // No more action needed
            }
        }
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
        case State::NewArrow:
            if(event.button.button == SDL_BUTTON_LEFT) {
                if(_nodeThatArrowMightConnectTo == _nodeThatIsTheSourceOfArrow) {
                    // Delete the arrow
                    deleteArrow(_arrowThatIsBeingCreated);
                    // Go to waiting state again
                    switchToStateWaiting();
                }
                else if(_nodeThatArrowMightConnectTo) {
                    // Attach the arrow to the node permanently
                    _arrowThatIsBeingCreated->finalizeCreation();
                    // Begin editing the label of the arrow TODO
                    switchToStateWaiting();
                }
                else {
                    // Create a new node, and connect the arrow to it TODO
                    // TODO the following lines are only a placeholder:
                    deleteArrow(_arrowThatIsBeingCreated);
                    // Go to waiting state again
                    switchToStateWaiting();
                }
            }
            break;
        }
        break;
    case SDL_MOUSEWHEEL:
        // No matter what state, scroll the view
        {
            fractionalPan.x += -event.wheel.preciseX * panSensitivity;
            fractionalPan.y += event.wheel.preciseY * panSensitivity;
            int x = fractionalPan.x;
            int y = fractionalPan.y;
            fractionalPan.x -= x;
            fractionalPan.y -= y;
            for(size_t i = 0; i < nodes.size(); i++) {
                nodes[i]->translate(x, y);
            }
            // Update arrow handle if needed
            if(_arrowHandle->isActive()) _arrowHandle->setToNode(_nodeThatHasArrowHandle);
            // Update the view
            redrawRequested = true;
            fullRedrawNeeded = true;
        }
        break;
    case SDL_WINDOWEVENT:
        redrawRequested = true;
        fullRedrawNeeded = true;
        // TODO only request redraw when actually needed
        break;
    default:
        // Ignore event
        break;
    }
}

void View::_render(SDL_Renderer* renderer) {
    // If full redraw is needed, redraw everything
    if(fullRedrawNeeded) {
        fullRedrawNeeded = false;
        // Draw background
        SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
        SDL_Rect screenRect;
        screenRect.x = 0;
        screenRect.y = 0;
        SDL_GetWindowSize(window, &screenRect.w, &screenRect.h);
        SDL_RenderFillRect(renderer, &screenRect);
        // Draw arrow handle
        SDL_Rect none;
        _arrowHandle->getOverlapRect(none); // reset overlap rect
        if(_arrowHandle->isActive()) {
            drawChild(*_arrowHandle);
        }
        // Render arrows
        for(size_t i = 0; i < arrows.size(); i++) {
            // Reset arrow's overlap rect
            arrows[i]->arrowCurve->getOverlapRect();
            // Redraw arrow curve
            drawChild(*arrows[i]->arrowCurve);
        }
        // Render nodes
        for(size_t i = 0; i < nodes.size(); i++) {
            // Reset node's requested overlap rect
            nodes[i]->getOverlapRect();
            // Redraw node
            drawChild(*nodes[i]);
        }
        return;
    }

    // Otherwise, do a more efficient redraw:

    // TODO remove off-screen overlap rects
    // Draw background behind overlap rects
    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
    for(size_t i = 0; i < overlapRects.size(); i++) {
        SDL_RenderFillRect(renderer, &overlapRects[i]);
    }
    // Draw arrow handle
    if(_arrowHandle->isActive()) {
        if(_arrowHandle->requestsToBeRedrawn()) {
            drawChild(*_arrowHandle);
            overlapRects.push_back(_arrowHandle->getRect());
            SDL_Rect rect = _arrowHandle->getRect();
        }
        else {
            for(size_t j = 0; j < overlapRects.size(); j++) {
                drawChildIntoClipRect(*_arrowHandle, overlapRects[j]);
            }
        }
    }
    // Render arrow curves
    for(size_t i = 0; i < arrows.size(); i++) {
        // Redraw arrow curve if it needs to
        if(arrows[i]->arrowCurve->requestsToBeRedrawn() || arrows[i]->arrowCurve->hasMoved()) {
            drawChild(*arrows[i]->arrowCurve);
            overlapRects.push_back(arrows[i]->arrowCurve->getRect());
        }
        // Otherwise, only redraw sections of it that overlap other things
        // that returned an overlap rect
        else {
            SDL_Rect curveRect = arrows[i]->arrowCurve->getRect();
            for(size_t j = 0; j < overlapRects.size(); j++) {
                // If the curve overlapps the rect, draw it again
                drawChildIntoClipRect(*arrows[i]->arrowCurve, overlapRects[j]);
            }
        }
    }
    // Render nodes
    for(size_t i = 0; i < nodes.size(); i++) {
        // Redraw node if it needs to
        if(nodes[i]->requestsToBeRedrawn() || nodes[i]->hasMoved()) {
            drawChild(*nodes[i]);
            overlapRects.push_back(nodes[i]->getRect());
        }
        // Otherwise, only redraw sections of it that overlap other things that
        // returned an overlap rect
        else {
            SDL_Rect nodeRect = nodes[i]->getRect();
            for(size_t j = 0; j < overlapRects.size(); j++) {
                // If the node overlapps the rect, draw it again
                drawChildIntoClipRect(*nodes[i], overlapRects[j]);
            }
        }
    }
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

void View::arrowHandleSystemOn(bool isIt) {
    // TODO don't break arrow handle system inbetween states
    _arrowHandleSystemIsOn = isIt;
    if(_arrowHandleSystemIsOn) {
        _nodeThatHasArrowHandle = getNodeAtMouse();
        _arrowHandle->setToNode(_nodeThatHasArrowHandle);
    }
    else {
        _nodeThatHasArrowHandle = nullptr;
        _arrowHandle->reset();
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
    arrowHandleSystemOn(false);
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
    clearSelection();
    _nodeThatIsBeingInteractedWith = nodeThatWasClickedOn;
    _nodeThatIsBeingInteractedWith->startInteraction();
}

void View::switchToStateNewArrow(std::shared_ptr<Node> sourceNode) {
    resetState();
    state = State::NewArrow;
    arrowHandleSystemOn(false);
    // Make a new arrow
    _nodeThatIsTheSourceOfArrow = sourceNode;
    _arrowThatIsBeingCreated = std::shared_ptr<Arrow>(new Arrow(renderer, sourceNode, mousePosition));
    arrows.push_back(_arrowThatIsBeingCreated);
    _nodeThatIsTheSourceOfArrow->addOutgoingArrow(_arrowThatIsBeingCreated);
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
        arrowHandleSystemOn(true);
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
    case State::NewArrow:
        _nodeThatIsTheSourceOfArrow = nullptr;
        _arrowThatIsBeingCreated = nullptr;
        _nodeThatArrowMightConnectTo = nullptr;
        arrowHandleSystemOn(true);
        break;
    /*
    case State::Selecting:
        //selectionBox.style.visibility = "collapse"; TODO
        _mousePositionAtStartOfSelection = nullptr;
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

void View::deleteSelectedNodes() {
    for(size_t i = 0; i < nodes.size(); i++) {
        for(size_t j = 0; j < selectedNodes.size(); j++) {
            if(selectedNodes[j] == nodes[i]) {
                auto arrowsConnectedToNode = nodes[i]->getAllArrows();
                for(size_t k = 0; k < arrowsConnectedToNode.size(); k++) {
                    std::shared_ptr arrow{arrowsConnectedToNode[k].lock()};
                    deleteArrow(arrow);
                }
                // TODO NOW remove arrow handle from node
                nodes.erase(nodes.begin() + i--);
                break;
            }
        }
    }
    selectedNodes.clear();
    redrawRequested = true;
    fullRedrawNeeded = true;
}

void View::deleteArrow(std::shared_ptr<Arrow> arrow) {
    for(size_t i = 0; i < arrows.size(); i++) {
        if(arrows[i] == arrow) {
            arrows.erase(arrows.begin() + i);
            redrawRequested = true;
            fullRedrawNeeded = true;
            return;
        }
    }
    SDL_Log("Error: tried to delete an arrow that wasn't on the list");
}
