#pragma once

#include <SDL.h>
#include <string>
#include "Arrow.h"
#include "ArrowTerminal.h"

class Node : public ArrowTerminal
{
public:
    Node(SDL_Renderer* renderer);
    void translate(int dx, int dy);
    bool isSelected();
    void isSelected(bool isIt);
    bool isHovered();
    void isHovered(bool isIt);
    SDL_Rect getOverlapRect();
    virtual void startInteraction() {} // override this
    virtual void stopInteraction() {} // override this
    virtual void handleEvent(const SDL_Event& event) {} // override this
    virtual std::string toString() = 0; // override this
    virtual bool isEmpty() = 0; // override this
    virtual ~Node() = default;

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
