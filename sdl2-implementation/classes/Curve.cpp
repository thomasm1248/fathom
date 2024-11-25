#include "Curve.h"
#include "Util.h"

Curve::Curve()
{
}

Curve::Curve(SDL_FPoint source, SDL_FPoint control, SDL_FPoint target)
    : source(source)
    , control(control)
    , target(target)
{
}

SDL_FPoint Curve::bezier(float t) {
    float tx0 = Util::lerp(source.x, control.x, t);
    float ty0 = Util::lerp(source.y, control.y, t);
    float tx1 = Util::lerp(control.x, target.x, t);
    float ty1 = Util::lerp(control.y, target.y, t);
    SDL_FPoint p;
    p.x = Util::lerp(tx0, tx1, t);
    p.y = Util::lerp(ty0, ty1, t);
    return p;
}

Curve Curve::offset(float offset) {
    Curve newCurve;
    // Find the new start point
    SDL_FPoint start = Util::normalize(Util::subtract(control, source));
    float temp = start.x;
    start.x = -start.y;
    start.y = temp;
    newCurve.source = Util::add(source, Util::scale(start, offset));
    // Find the new end point
    SDL_FPoint end = Util::normalize(Util::subtract(target, control));
    temp = end.x;
    end.x = -end.y;
    end.y = temp;
    newCurve.target = Util::add(target, Util::scale(end, offset));
    // Find new control point
    SDL_FPoint n = Util::add(start, end);
    newCurve.control = Util::add(control, Util::scale(n, 2*offset/Util::dot(n,n)));
    return newCurve;
}

void Curve::translate(SDL_FPoint shift) {
    source = Util::add(source, shift);
    control = Util::add(control, shift);
    target = Util::add(target, shift);
}
