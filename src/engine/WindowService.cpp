#include <engine/WindowService.hpp>
#include <log/log.hpp>

namespace lw
{

WindowService::WindowService(std::string_view title, int width, int height)
{
    m_logger = create_logger("WindowService");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        log_fatal_and_throw(m_logger, "SDL_Init Failed: " + std::string(SDL_GetError()));
    }

    m_window = SDL_CreateWindow(title.data(), width, height, SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        log_fatal_and_throw(m_logger, "SDL_CreateWindow Failed: " + std::string(SDL_GetError()));
    }

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer)
    {
        log_fatal_and_throw(m_logger, "SDL_CreateRenderer Failed: " + std::string(SDL_GetError()));
    }

    SDL_SetRenderVSync(m_renderer, 1);
    m_logger->info("Window '{}' ({}x{}) created", title, width, height);
}

WindowService::~WindowService()
{
    if (m_renderer)
        SDL_DestroyRenderer(m_renderer);
    if (m_window)
        SDL_DestroyWindow(m_window);
    SDL_Quit();
}

} // namespace lw
