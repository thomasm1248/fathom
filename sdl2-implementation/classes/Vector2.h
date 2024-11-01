#pragma once

class Vector2
{
public:
    float x = 0;
    float y = 0;

    Vector2() {}
    Vector2(float x, float y);
    Vector2& operator+=(Vector2 const& rhs);
    Vector2 operator+(Vector2 const& rhs);
};
