#include "Vector2.h"

Vector2::Vector2(float x, float y)
    : x(x)
    , y(y)
{}

Vector2& Vector2::operator+=(Vector2 const& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
}

Vector2 Vector2::operator+(Vector2 const& rhs) {
    Vector2 temp(x + rhs.x, y + rhs.y);
    return temp;
}
