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

void ArrowTerminal::removeOutgoingArrow(std::weak_ptr<Arrow> arrow) {
    std::shared_ptr _arrow{arrow.lock()};
    for(size_t i = 0; i < outgoingArrows.size(); i++) {
        std::shared_ptr outgoingArrow{outgoingArrows[i].lock()};
        if(outgoingArrow == _arrow) {
            outgoingArrows.erase(outgoingArrows.begin() + i);
            return;
        }
    }
}

void ArrowTerminal::removeIncomingArrow(std::weak_ptr<Arrow> arrow) {
    std::shared_ptr _arrow{arrow.lock()};
    for(size_t i = 0; i < incomingArrows.size(); i++) {
        std::shared_ptr incomingArrow{incomingArrows[i].lock()};
        if(incomingArrow == _arrow) {
            incomingArrows.erase(incomingArrows.begin() + i--);
            return;
        }
    }
}

std::vector<std::weak_ptr<Arrow>> ArrowTerminal::getAllArrows() {
    std::vector<std::weak_ptr<Arrow>> fullList;
    fullList.insert(fullList.end(), outgoingArrows.begin(), outgoingArrows.end());
    fullList.insert(fullList.end(), incomingArrows.begin(), incomingArrows.end());
    // Remove expired arrows
    for(size_t i = 0; i < fullList.size(); i++) {
        if(fullList[i].expired()) {
            fullList.erase(fullList.begin() + i--);
        }
    }
    return fullList;
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
