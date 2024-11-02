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
    SDL_Point mousePosition;
    SDL_Point mouseVelocity;
    bool shiftIsPressed = false; // TODO update this from events
    //std::string filepath;
    //File file;
    std::vector<std::shared_ptr<Node>> nodes;
    //std::vector<Arrow> arrows;
    std::vector<SDL_Rect> overlapRects;

    void _render(SDL_Renderer* renderer);



    // State machine

    void switchToStateWaiting();

    void switchToStateDragging(std::shared_ptr<Node> nodeToBeDragged, bool shiftButtonIsPressed);
    bool _userMightBeTryingToInteractWithNode;
    std::shared_ptr<Node> _nodeThatIsDirectTargetOfDrag;

    void resetState();

    /*
    Node* _nodeThatIsBeingHovered;
    Node* _nodeThatWasMostRecentlySelected;



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


    bool _arrowHandleSystemIsOn;
    Node* _nodeThatHasArrowHandle;
    void turnOnArrowHandleSystem();
    void turnOffArrowHandleSystem();*/
    


    // Util functions
    std::shared_ptr<Node> getNodeAtMouse();
    void moveNodeToFront(std::shared_ptr<Node> node);
};