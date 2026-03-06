#ifndef LOOMWOOD_PHYSICS_SYSTEM_HPP
#define LOOMWOOD_PHYSICS_SYSTEM_HPP

#include <entt/entt.hpp>

namespace lw
{

class PhysicsSystem
{
public:
    void update(entt::registry& registry, float dt);
};

} // namespace lw

#endif // LOOMWOOD_PHYSICS_SYSTEM_HPP
