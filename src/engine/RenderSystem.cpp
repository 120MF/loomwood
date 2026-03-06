#include <engine/RenderSystem.hpp>
#include <engine/WindowService.hpp>
#include <engine/components.hpp>
#include <entt/entt.hpp>

namespace lw
{

void RenderSystem::render(entt::registry& registry)
{
    auto& window = entt::locator<WindowService>::value();
    SDL_Renderer* renderer = window.renderer();

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    auto view = registry.view<TransformComponent, ColorComponent>();
    for (auto entity : view)
    {
        auto& transform = view.get<TransformComponent>(entity);
        auto& color     = view.get<ColorComponent>(entity);

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_FRect rect = { transform.x, transform.y, transform.width, transform.height };
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
}

} // namespace lw
