#pragma once

#include <SDL.h>
#include "Node.h"
#include "Vector2.h"
#include "Renderable.h"
#include <vector>
#include <memory>
#include "ViewFile.h"
#include "ArrowHandle.h"
#include "SelectionBox.h"
#include "Font.h"

class View : public Renderable
{
public:
    View(SDL_Renderer* renderer, SDL_Window* window, std::shared_ptr<ViewFile> viewFile, std::shared_ptr<Font> labelNodeFont);
    void handleEvent(const SDL_Event& event);
    void processDynamicContent();
    void checkWhatNeedsToBeRedrawn();

private:
    static constexpr int panSensitivity = 10;
    static constexpr int arrowHandleWidth = 20;

    SDL_Window* window;
    SDL_Renderer* renderer;
    std::shared_ptr<ViewFile> viewFile;
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
    std::vector<std::shared_ptr<Node>> nodes;
    std::vector<std::shared_ptr<Node>> selectedNodes;
    std::vector<std::shared_ptr<Arrow>> arrows;
    std::vector<SDL_Rect> overlapRects;
    bool fullRedrawNeeded = true;
    SDL_FPoint fractionalPan{0.0,0.0};
    std::shared_ptr<Font> labelNodeFont;

    void _render(SDL_Renderer* renderer);



    // State machine

    // shared
    void resetState();

    // System: Hovering
    void hoveringSystemOn(bool isIt);
    bool _hoveringSystemIsOn = true;
    std::shared_ptr<Node> _nodeThatIsBeingHovered;

    // System: Arrow Handle
    void arrowHandleSystemOn(bool isIt);
    bool _arrowHandleSystemIsOn = true;
    std::shared_ptr<Node> _nodeThatHasArrowHandle;
    std::shared_ptr<ArrowHandle> _arrowHandle;

    // Waiting
    void switchToStateWaiting();

    // Dragging
    void switchToStateDragging(std::shared_ptr<Node> nodeToBeDragged);
    bool _userMightBeTryingToInteractWithNode;
    std::shared_ptr<Node> _nodeThatIsDirectTargetOfDrag;

    // Interacting
    void switchToStateInteracting(std::shared_ptr<Node> nodeThatWasClickedOn);
    std::shared_ptr<Node> _nodeThatIsBeingInteractedWith;

    // New Arrow
    void switchToStateNewArrow(std::shared_ptr<Node> sourceNode);
    std::shared_ptr<Node> _nodeThatIsTheSourceOfArrow;
    std::shared_ptr<Arrow> _arrowThatIsBeingCreated;
    std::shared_ptr<Node> _nodeThatArrowMightConnectTo;

    // Selecting
    void switchToStateSelecting(SDL_Point startPoint);
    std::shared_ptr<SelectionBox> _selectionBox;

    /*
    Node* _nodeThatWasMostRecentlySelected;





    //Arrow* _arrowThatIsBeingDragged: Arrow = null; TODO
    float _tValueOfArrowBeingDragged;
    void switchToStateDraggingArrow();

    //Arrow* _arrowThatIsBeingInteractedWith TODO
    void switchToStateInteractingWithArrow();

    */
    


    // Util functions
    std::shared_ptr<Node> getNodeAtMouse();
    void moveNodeToFront(std::shared_ptr<Node> node);
    void clearSelection();
    void addNodeToSelection(std::shared_ptr<Node> node);
    void removeNodeFromSelection(std::shared_ptr<Node> node);
    void addNodesToSelection(std::vector<std::shared_ptr<Node>>&& nodesToAdd);
    void deleteSelectedNodes();
    void deleteNode(std::shared_ptr<Node> node);
    void deleteArrow(std::shared_ptr<Arrow> arrow);
};
