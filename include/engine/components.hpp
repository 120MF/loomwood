#ifndef LOOMWOOD_COMPONENTS_HPP
#define LOOMWOOD_COMPONENTS_HPP

#include <cstdint>

namespace lw
{

struct TransformComponent
{
    float x, y;
    float width, height;
};

struct VelocityComponent
{
    float dx, dy;
};

struct ColorComponent
{
    uint8_t r, g, b, a;
};

} // namespace lw

#endif // LOOMWOOD_COMPONENTS_HPP
