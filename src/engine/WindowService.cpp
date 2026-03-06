#include <engine/WindowService.hpp>
#include <log/log.hpp>
#include <toml++/toml.hpp>

namespace lw
{
    static constexpr std::string_view WINDOW_TITLE = "The Loomwood Crafter";

    WindowService::WindowService()
    {
        m_logger = create_logger("WindowService");

        auto tbl = toml::parse_file("configs/window.toml");
        int width = tbl["window"]["width"].value_or(800);
        int height = tbl["window"]["height"].value_or(600);

        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            log_fatal_and_throw(m_logger, "SDL_Init Failed: " + std::string(SDL_GetError()));
        }

        m_window = SDL_CreateWindow(WINDOW_TITLE.data(), width, height,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
        if (!m_window)
        {
            log_fatal_and_throw(m_logger, "SDL_CreateWindow Failed: " + std::string(SDL_GetError()));
        }

        m_renderer = SDL_CreateRenderer(m_window, nullptr);
        if (!m_renderer)
        {
            log_fatal_and_throw(m_logger, "SDL_CreateRenderer Failed: " + std::string(SDL_GetError()));
        }

        m_designWidth  = tbl["window"]["design_width"].value_or(1920);
        m_designHeight = tbl["window"]["design_height"].value_or(1080);

        SDL_SetRenderVSync(m_renderer, 1);

        // NEAREST 让 letterbox 中间纹理不引入额外模糊；字体锐度由
        // dp-ratio 控制，而非依赖双线性插值。
        SDL_SetDefaultTextureScaleMode(m_renderer, SDL_SCALEMODE_NEAREST);
        SDL_SetRenderLogicalPresentation(
            m_renderer,
            m_designWidth, m_designHeight,
            SDL_LOGICAL_PRESENTATION_LETTERBOX);
        SDL_SetDefaultTextureScaleMode(m_renderer, SDL_SCALEMODE_LINEAR);

        m_logger->info("Window '{}' ({}x{}) created, logical {}x{}",
            WINDOW_TITLE, width, height, m_designWidth, m_designHeight);
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
