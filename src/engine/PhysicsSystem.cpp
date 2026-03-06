#include <engine/PhysicsSystem.hpp>
#include <engine/components.hpp>

namespace lw
{

void PhysicsSystem::update(entt::registry& registry, float dt)
{
    auto view = registry.view<TransformComponent, VelocityComponent>();
    for (auto entity : view)
    {
        auto& transform = view.get<TransformComponent>(entity);
        auto& velocity  = view.get<VelocityComponent>(entity);

        transform.x += velocity.dx * dt;
        transform.y += velocity.dy * dt;

        // 边界反弹
        if (transform.x <= 0.0f || transform.x + transform.width >= 800.0f)
            velocity.dx *= -1.0f;
        if (transform.y <= 0.0f || transform.y + transform.height >= 600.0f)
            velocity.dy *= -1.0f;
    }
}

} // namespace lw
