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

protected:
    virtual void _selectedStatusHasChanged(bool isItSelected); // override this
    virtual void _hoveredStatusHasChanged(bool isItHovered); // override this

private:
    bool _isSelected = false;
    bool _isHovered = false;
};
