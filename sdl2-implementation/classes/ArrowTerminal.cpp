#include "ArrowTerminal.h"
#include "Arrow.h"

SDL_FPoint ArrowTerminal::getCenter() {
    SDL_Rect rect = getRect();
    return {rect.x + rect.w/2.f, rect.y + rect.h/2.f};
}

void ArrowTerminal::addOutgoingArrow(std::weak_ptr<Arrow> arrow) {
    outgoingArrows.push_back(arrow);
}

void ArrowTerminal::addIncomingArrow(std::weak_ptr<Arrow> arrow) {
    incomingArrows.push_back(arrow);
}

void ArrowTerminal::removeArrow(std::weak_ptr<Arrow> arrow) {
    std::shared_ptr _arrow{arrow.lock()};
    for(size_t i = 0; i < outgoingArrows.size(); i++) {
        std::shared_ptr outgoingArrow{outgoingArrows[i].lock()};
        if(outgoingArrow == _arrow) {
            outgoingArrows.erase(outgoingArrows.begin() + i);
            return;
        }
    }
    for(size_t i = 0; i < incomingArrows.size(); i++) {
        std::shared_ptr incomingArrow{incomingArrows[i].lock()};
        if(incomingArrow == _arrow) {
            incomingArrows.erase(incomingArrows.begin() + i--);
            return;
        }
    }
}

void ArrowTerminal::translateArrows() {
    for(size_t i = 0; i < outgoingArrows.size(); i++) {
        if(outgoingArrows[i].expired())
            outgoingArrows.erase(outgoingArrows.begin() + i--);
        else {
            std::shared_ptr arrow{outgoingArrows[i].lock()};
            arrow->updateSource();
        }
    }
    for(size_t i = 0; i < incomingArrows.size(); i++) {
        if(incomingArrows[i].expired())
            incomingArrows.erase(incomingArrows.begin() + i--);
        else {
            std::shared_ptr arrow{incomingArrows[i].lock()};
            arrow->updateTarget();
        }
    }
}
