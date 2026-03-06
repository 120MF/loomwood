#ifndef LOOMWOOD_WINDOW_SERVICE_HPP
#define LOOMWOOD_WINDOW_SERVICE_HPP

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <string_view>
#include <memory>

namespace lw
{

class WindowService
{
public:
    WindowService(std::string_view title, int width, int height);
    ~WindowService();

    [[nodiscard]] SDL_Window*   window()   const noexcept { return m_window; }
    [[nodiscard]] SDL_Renderer* renderer() const noexcept { return m_renderer; }

private:
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    std::shared_ptr<spdlog::logger> m_logger;
};

} // namespace lw

#endif // LOOMWOOD_WINDOW_SERVICE_HPP
