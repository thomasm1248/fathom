#pragma once

#include "Renderable.h"
#include <vector>
#include <memory>

class Arrow;

class ArrowTerminal : public Renderable
{
public:
    ArrowTerminal(SDL_Renderer* renderer) : Renderable(renderer) {}
    SDL_FPoint getCenter();
    void addOutgoingArrow(std::weak_ptr<Arrow> arrow);
    void addIncomingArrow(std::weak_ptr<Arrow> arrow);
    void removeOutgoingArrow(std::weak_ptr<Arrow> arrow);
    void removeIncomingArrow(std::weak_ptr<Arrow> arrow);
    std::vector<std::weak_ptr<Arrow>> getAllArrows();

protected:
    void translateArrows();

private:
    std::vector<std::weak_ptr<Arrow>> outgoingArrows; // TODO delete arrows when node is deleted
    std::vector<std::weak_ptr<Arrow>> incomingArrows;
};
