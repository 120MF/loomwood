#include <engine/Engine.hpp>
#include <engine/WindowService.hpp>
#include <engine/InputSystem.hpp>
#include <engine/PhysicsSystem.hpp>
#include <engine/RenderSystem.hpp>
#include <engine/components.hpp>
#include <log/log.hpp>

#include <SDL3/SDL.h>

namespace lw
{

Engine::Engine()
{
    init_logging();
    m_logger = create_logger("Engine");

    entt::locator<WindowService>::emplace<WindowService>();
    entt::locator<InputSystem>::emplace<InputSystem>();
    entt::locator<PhysicsSystem>::emplace<PhysicsSystem>();
    entt::locator<RenderSystem>::emplace<RenderSystem>();

    m_lastTime = SDL_GetTicksNS();
    m_isRunning = true;
    initGameObjects();
    m_logger->info("Engine initialized");
}

Engine::~Engine()
{
    // 逆序销毁，确保 WindowService 最后释放 SDL 资源
    entt::locator<RenderSystem>::reset();
    entt::locator<PhysicsSystem>::reset();
    entt::locator<InputSystem>::reset();
    entt::locator<WindowService>::reset();
}

void Engine::run()
{
    while (m_isRunning)
    {
        entt::locator<InputSystem>::value().process(m_isRunning);

        uint64_t current_time = SDL_GetTicksNS();
        float dt = static_cast<float>(current_time - m_lastTime) / 1'000'000'000.0f;
        m_lastTime = current_time;

        entt::locator<PhysicsSystem>::value().update(m_registry, dt);
        entt::locator<RenderSystem>::value().render(m_registry);
    }
}

void Engine::initGameObjects()
{
    auto dragon = m_registry.create();
    m_registry.emplace<TransformComponent>(dragon, 100.0f, 100.0f, 50.0f, 50.0f);
    m_registry.emplace<VelocityComponent>(dragon, 150.0f, 100.0f);
    m_registry.emplace<ColorComponent>(dragon, uint8_t{255}, uint8_t{255}, uint8_t{255}, uint8_t{255});

    auto npc = m_registry.create();
    m_registry.emplace<TransformComponent>(npc, 400.0f, 300.0f, 40.0f, 40.0f);
    m_registry.emplace<ColorComponent>(npc, uint8_t{100}, uint8_t{200}, uint8_t{100}, uint8_t{255});
}

} // namespace lw

