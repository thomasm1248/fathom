#include "SelectionBox.h"

SelectionBox::SelectionBox(SDL_Renderer* renderer)
    : Renderable(renderer)
{
    initializeTexture(1, 1);
}

bool SelectionBox::visible() {
    return _visible;
}

bool SelectionBox::getOverlapRect(SDL_Rect& oRect) {
    if(hasOverlapRect) {
        hasOverlapRect = false;
        oRect = overlapRect;
        return true;
    }
    return false;
}

void SelectionBox::beginSelection(SDL_Point start) {
    _visible = true;
    this->start = start;
    this->end = start;
    recalculateDimensions();
}

void SelectionBox::continueSelection(SDL_Point end) {
    if(!hasOverlapRect) {
        overlapRect = getRect();
        hasOverlapRect = true;
    }
    this->end = end;
    recalculateDimensions();
}

std::vector<std::shared_ptr<Node>> SelectionBox::finishSelection(std::vector<std::shared_ptr<Node>> nodes) {
    // If box is too small to be visible, then it couldn't have selected anything
    if(!_visible) {
        return {};
    }
    // Make invisible
    if(!hasOverlapRect) {
        overlapRect = getRect();
        hasOverlapRect = true;
    }
    _visible = false;
    // Find which nodes have been selected
    std::vector<std::shared_ptr<Node>> selectedNodes;
    SDL_Rect selectionRect = getRect();
    for(size_t i = 0; i < nodes.size(); i++) {
        // Check if the node is completely within the selection box
        SDL_Rect nodeRect = nodes[i]->getRect();
        if(nodeRect.x < selectionRect.x) continue;
        if(nodeRect.y < selectionRect.y) continue;
        if(nodeRect.x + nodeRect.w > selectionRect.x + selectionRect.w) continue;
        if(nodeRect.y + nodeRect.h > selectionRect.y + selectionRect.h) continue;
        selectedNodes.push_back(nodes[i]);
    }
    return selectedNodes;
}

void SelectionBox::cancelSelection() {
    if(!_visible) return;
    // Make invisible
    if(!hasOverlapRect) {
        overlapRect = getRect();
        hasOverlapRect = true;
    }
    _visible = false;
}

void SelectionBox::_render(SDL_Renderer* renderer) {
    // Get the dimensions
    auto rect = getRect();
    rect.x = 0;
    rect.y = 0;
    // Draw the rect
    SDL_SetRenderDrawColor(renderer, 76, 219, 212, 128);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 76, 219, 212, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

void SelectionBox::recalculateDimensions() {
    auto oldRect = getRect();
    SDL_Rect dimensions;
    // Calculate position
    dimensions.x = start.x > end.x ? end.x : start.x;
    dimensions.y = start.y > end.y ? end.y : start.y;
    // Calculate width & height
    dimensions.w = start.x - end.x;
    dimensions.h = start.y - end.y;
    if(dimensions.w < 0) dimensions.w *= -1;
    if(dimensions.h < 0) dimensions.h *= -1;
    // If box has no area, make invisible instead
    if(SDL_RectEmpty(&dimensions)) {
        if(!hasOverlapRect) {
            overlapRect = oldRect;
            hasOverlapRect = true;
        }
        _visible = false;
        return;
    }
    else {
        _visible = true;
    }
    // Set location and dimensions to calculated measurements
    moveTexture(dimensions.x, dimensions.y);
    resizeTexture(dimensions.w, dimensions.h);
}
