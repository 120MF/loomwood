#include <engine/WindowService.hpp>
#include <log/log.hpp>
#include <toml++/toml.hpp>

namespace lw
{

static constexpr std::string_view WINDOW_TITLE = "The Loomwood Crafter";

WindowService::WindowService()
{
    m_logger = create_logger("WindowService");

    auto tbl   = toml::parse_file("config/window.toml");
    int  width  = tbl["window"]["width"].value_or(800);
    int  height = tbl["window"]["height"].value_or(600);

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        log_fatal_and_throw(m_logger, "SDL_Init Failed: " + std::string(SDL_GetError()));
    }

    m_window = SDL_CreateWindow(WINDOW_TITLE.data(), width, height, SDL_WINDOW_RESIZABLE);
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
    m_logger->info("Window '{}' ({}x{}) created", WINDOW_TITLE, width, height);
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
