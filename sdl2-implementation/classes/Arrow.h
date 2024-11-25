#pragma once

#include <SDL.h>
#include <memory>
#include "ArrowCurve.h"
#include "ArrowTerminal.h"
#include <string>

class Arrow
{
public:
    std::shared_ptr<ArrowCurve> arrowCurve;

    Arrow(SDL_Renderer* renderer, std::shared_ptr<ArrowTerminal> sourceNode, SDL_Point mousePosition);
    Arrow(SDL_Renderer* renderer, std::shared_ptr<ArrowTerminal> sourceNode, std::shared_ptr<ArrowTerminal> targetNode, std::string label, SDL_FPoint controlPoint);
    void updateEndFromMousePosition(SDL_Point mousePosition);
    void attachToNode(std::shared_ptr<ArrowTerminal> targetNode);
    void updateSource();
    void updateTarget();
    std::string getLabel();
    std::shared_ptr<ArrowTerminal> getSourceNode();
    std::shared_ptr<ArrowTerminal> getTargetNode();

private:
    std::shared_ptr<ArrowTerminal> sourceNode;
    SDL_FPoint controlPoint;
    std::shared_ptr<ArrowTerminal> targetNode;
    std::string label;

    

    // State machine
    enum class State {
        Creating,
        Nothing,
        Selected,
        Interacting
    };
    State state = State::Nothing;
    void resetState();

    // Creating
    void switchToStateCreating(SDL_Point mousePosition);
    SDL_Point _mousePosition;

    // Nothing

    // Selected

    // Interacting
};
