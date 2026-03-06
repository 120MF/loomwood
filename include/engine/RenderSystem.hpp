#ifndef LOOMWOOD_RENDER_SYSTEM_HPP
#define LOOMWOOD_RENDER_SYSTEM_HPP

#include <entt/entt.hpp>

namespace lw
{

class RenderSystem
{
public:
    void render(entt::registry& registry);
};

} // namespace lw

#endif // LOOMWOOD_RENDER_SYSTEM_HPP
