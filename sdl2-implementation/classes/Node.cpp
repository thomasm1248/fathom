#include "Node.h"

void Node::translate(int dx, int dy) {
    auto rect = getRect();
    rect.x += dx;
    rect.y += dy;
    moveTexture(rect.x, rect.y);
}
