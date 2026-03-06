#include <engine/InputSystem.hpp>
#include <engine/UiSystem.hpp>
#include <engine/WindowService.hpp>
#include <entt/entt.hpp>
#include <SDL3/SDL.h>

namespace lw
{

void InputSystem::process(bool& isRunning)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            isRunning = false;
            break;

        case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_ESCAPE)
                isRunning = false;
            break;

        case SDL_EVENT_WINDOW_RESIZED:
        {
            // 物理窗口尺寸变化时，SDL 的逻辑呈现会自动重新缩放。
            // 同步更新 RmlUi 上下文尺寸，保持 UI 在逻辑坐标系中正确工作。
            auto& win = entt::locator<WindowService>::value();
            auto* ctx = entt::locator<UiSystem>::value().context();
            if (ctx)
                ctx->SetDimensions({ win.designWidth(), win.designHeight() });
            break;
        }

        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
        {
            // 显示器 DPI 变化（如窗口移到另一个屏幕）时重新设置 dp-ratio，
            // RmlUi 会以新密度重新栅格化所有字体和 dp 单位尺寸。
            auto& win = entt::locator<WindowService>::value();
            const float ratio = SDL_GetWindowDisplayScale(win.window());
            entt::locator<UiSystem>::value().setDpRatio(ratio);
            break;
        }

        default:
            break;
        }
    }
}

} // namespace lw

