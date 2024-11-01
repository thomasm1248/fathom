#pragma once

#include <SDL.h>
#include "Node.h"
#include "Vector2.h"
#include "Renderable.h"
#include <vector>
#include <memory>

class View : public Renderable
{
public:
    View(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event);

private:
    enum class State {
        Waiting,
        //Panning, TODO
        Dragging,
        Selecting,
        Interacting,
        NewArrow,
        DraggingArrow,
        InteractingWithArrow
    };
    State state = State::Waiting;
    //std::string filepath;
    //File file;
    std::vector<std::shared_ptr<Node>> nodes;
    //std::vector<Arrow> arrows;
    std::vector<SDL_Rect> overlapRects;

    void _render(SDL_Renderer* renderer);

    // State machine
    /*
    Node* _nodeThatIsBeingHovered;
    Node* _nodeThatWasMostRecentlySelected;

    void switchToStateWaiting();

    bool _userMightBeTryingToInteractWithNode;
    Node* _nodeThatIsDirectTargetOfDrag;
    void switchToStateDragging(Node* nodeToBeDragged, bool shiftButtonIsPressed);

    Vector2 _mousePositionAtStartOfSelection;
    void switchToStateSelecting();

    Node* _nodeThatIsBeingInteractedWith;
    void switchToStateInteracting();

    //Arrow* _arrowThatIsBeingCreated TODO
    Node* _nodeThatArrowMightConnectTo;
    void switchToStateNewArrow();

    //Arrow* _arrowThatIsBeingDragged: Arrow = null; TODO
    float _tValueOfArrowBeingDragged;
    void switchToStateDraggingArrow();

    //Arrow* _arrowThatIsBeingInteractedWith TODO
    void switchToStateInteractingWithArrow();

    void resetState();

    bool _arrowHandleSystemIsOn;
    Node* _nodeThatHasArrowHandle;
    void turnOnArrowHandleSystem();
    void turnOffArrowHandleSystem();*/
};
