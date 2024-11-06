#pragma once

#include <SDL.h>
#include "Renderable.h"

class Node : public Renderable
{
public:
    Node(SDL_Renderer* renderer) : Renderable(renderer) {}
    void translate(int dx, int dy);
    bool isSelected();
    void isSelected(bool isIt);
    bool isHovered();
    void isHovered(bool isIt);
    SDL_Rect getOverlapRect();
    virtual void startInteraction() {} // override this
    virtual void stopInteraction() {} // override this
    virtual void handleEvent(const SDL_Event& event) {} // override this

protected:
    virtual void _selectedStatusHasChanged(bool isItSelected); // override this
    virtual void _hoveredStatusHasChanged(bool isItHovered); // override this
    void createOverlapRect();

private:
    bool _isSelected = false;
    bool _isHovered = false;
    bool hasOverlapRect = false;
    SDL_Rect overlapRect;
};
