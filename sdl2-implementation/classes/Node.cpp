#include "Node.h"

Node::Node(SDL_Renderer* renderer)
    : ArrowTerminal(renderer)
{
}

void Node::translate(int dx, int dy) {
    createOverlapRect();
    auto rect = getRect();
    rect.x += dx;
    rect.y += dy;
    moveTexture(rect.x, rect.y);
}

bool Node::isSelected() {
    return _isSelected;
}

void Node::isSelected(bool isIt) {
    createOverlapRect();
    _isSelected = isIt;
    _selectedStatusHasChanged(_isSelected);
}

bool Node::isHovered() {
    return _isHovered;
}

void Node::isHovered(bool isIt) {
    _isHovered = isIt;
    _hoveredStatusHasChanged(_isHovered);
}

SDL_Rect Node::getOverlapRect() {
    if(hasOverlapRect) {
        hasOverlapRect = false;
        translateArrows();
        return overlapRect;
    }
    return {0,0,0,0};
}

void Node::_selectedStatusHasChanged(bool isItSelected) {
    // Do nothing
    if(isItSelected) {
        SDL_Log("Node was selected, but isn't showing it graphically.");
    }
    else {
        SDL_Log("Node was deselected, but isn't showing it graphically.");
    }
}

void Node::_hoveredStatusHasChanged(bool isItHovered) {
    // Do nothing
    if(isItHovered) {
        SDL_Log("Node was hovered, but isn't showing it graphically.");
    }
    else {
        SDL_Log("Node was unhovered, but isn't showing it graphically.");
    }
}

void Node::createOverlapRect() {
    if(!hasOverlapRect) {
        overlapRect = getRect();
        hasOverlapRect = true;
    }
}
