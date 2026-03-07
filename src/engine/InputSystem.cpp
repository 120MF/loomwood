#include <engine/InputSystem.hpp>
#include <engine/UiSystem.hpp>
#include <engine/WindowService.hpp>
#include <log/log.hpp>
#include <entt/entt.hpp>
#include <SDL3/SDL.h>
#include <cmath>

namespace lw
{

InputSystem::InputSystem()
{
    m_logger = create_logger("InputSystem", spdlog::level::debug);

    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD))
        log_fatal_and_throw(m_logger, "SDL_InitSubSystem(GAMEPAD) failed: " + std::string(SDL_GetError()));
    else
        m_logger->info("InputSystem initialized (gamepad subsystem ready)");
}

InputSystem::~InputSystem()
{
    for (const auto& pad : m_gamepads | std::views::values)
        SDL_CloseGamepad(pad);
    m_gamepads.clear();
    SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
}

void InputSystem::process(bool& isRunning)
{
    m_input.resetTriggers();

    SDL_Event event;
    float rawAxisX = 0.0f;
    float rawAxisY = 0.0f;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        // ------------------------------------------------------------------
        // 系统事件
        // ------------------------------------------------------------------
        case SDL_EVENT_QUIT:
            m_logger->debug("[Event] SDL_EVENT_QUIT — stopping engine");
            isRunning = false;
            break;

        // ------------------------------------------------------------------
        // 窗口事件（保持与原实现一致）
        // ------------------------------------------------------------------
        case SDL_EVENT_WINDOW_RESIZED:
        {
            m_logger->debug("[Event] SDL_EVENT_WINDOW_RESIZED ({}x{})",
                event.window.data1, event.window.data2);
            auto& win = entt::locator<WindowService>::value();
            auto* ctx = entt::locator<UiSystem>::value().context();
            if (ctx)
                ctx->SetDimensions({ win.designWidth(), win.designHeight() });
            break;
        }

        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
        {
            auto& win       = entt::locator<WindowService>::value();
            const float ratio = SDL_GetWindowDisplayScale(win.window());
            m_logger->debug("[Event] SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED — dp-ratio={:.2f}", ratio);
            entt::locator<UiSystem>::value().setDpRatio(ratio);
            break;
        }

        // ------------------------------------------------------------------
        // 手柄热插拔
        // ------------------------------------------------------------------
        case SDL_EVENT_GAMEPAD_ADDED:
        {
            SDL_JoystickID id  = event.gdevice.which;
            SDL_Gamepad*   pad = SDL_OpenGamepad(id);
            if (pad)
            {
                m_gamepads[id] = pad;
                m_input.activeDevice = InputDeviceType::Gamepad;
                m_logger->info("[Event] SDL_EVENT_GAMEPAD_ADDED — '{}' (id={})",
                    SDL_GetGamepadName(pad), id);
            }
            else
            {
                m_logger->warn("[Event] SDL_EVENT_GAMEPAD_ADDED — SDL_OpenGamepad failed: {}", SDL_GetError());
            }
            break;
        }

        case SDL_EVENT_GAMEPAD_REMOVED:
        {
            SDL_JoystickID id = event.gdevice.which;
            if (m_gamepads.count(id))
            {
                SDL_CloseGamepad(m_gamepads[id]);
                m_gamepads.erase(id);
                m_input.activeDevice = InputDeviceType::KeyboardMouse;
                m_logger->info("[Event] SDL_EVENT_GAMEPAD_REMOVED — id={}, switched to KeyboardMouse", id);
            }
            break;
        }

        // ------------------------------------------------------------------
        // 键盘输入（任意按键切换至键鼠模式）
        // ------------------------------------------------------------------
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        {
            m_input.activeDevice = InputDeviceType::KeyboardMouse;
            const bool isDown = (event.type == SDL_EVENT_KEY_DOWN);
            const auto key    = event.key.key;

            m_logger->debug("[Event] {} key={} scancode={} repeat={}",
                isDown ? "SDL_EVENT_KEY_DOWN" : "SDL_EVENT_KEY_UP",
                SDL_GetKeyName(key), static_cast<int>(event.key.scancode), event.key.repeat);

            if (isDown && !event.key.repeat)
            {
                if (key == SDLK_W || key == SDLK_UP)
                {
                    m_input.uiUpJustPressed = true;
                    m_logger->debug("  -> uiUpJustPressed");
                }
                if (key == SDLK_S || key == SDLK_DOWN)
                {
                    m_input.uiDownJustPressed = true;
                    m_logger->debug("  -> uiDownJustPressed");
                }
                if (key == SDLK_RETURN || key == SDLK_SPACE)
                {
                    m_input.uiConfirmJustPressed = true;
                    m_logger->debug("  -> uiConfirmJustPressed");
                }
                if (key == SDLK_ESCAPE)
                {
                    m_input.uiCancelJustPressed = true;
                    m_logger->debug("  -> uiCancelJustPressed / quit");
                    isRunning = false;
                }
            }
            break;
        }

        // ------------------------------------------------------------------
        // 手柄按钮（任意按键切换至手柄模式）
        // ------------------------------------------------------------------
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        {
            m_input.activeDevice = InputDeviceType::Gamepad;
            const auto btn = event.gbutton.button;

            m_logger->debug("[Event] SDL_EVENT_GAMEPAD_BUTTON_DOWN — button={} ({})",
                btn, SDL_GetGamepadStringForButton(static_cast<SDL_GamepadButton>(btn)));

            if (btn == SDL_GAMEPAD_BUTTON_DPAD_UP)
            {
                m_input.uiUpJustPressed = true;
                m_logger->debug("  -> uiUpJustPressed");
            }
            if (btn == SDL_GAMEPAD_BUTTON_DPAD_DOWN)
            {
                m_input.uiDownJustPressed = true;
                m_logger->debug("  -> uiDownJustPressed");
            }
            if (btn == SDL_GAMEPAD_BUTTON_SOUTH)
            {
                m_input.uiConfirmJustPressed = true;
                m_logger->debug("  -> uiConfirmJustPressed (A/Cross)");
            }
            if (btn == SDL_GAMEPAD_BUTTON_EAST || btn == SDL_GAMEPAD_BUTTON_START)
            {
                m_input.uiCancelJustPressed = true;
                m_logger->debug("  -> uiCancelJustPressed (B/Circle or Start)");
            }
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            m_logger->debug("[Event] SDL_EVENT_GAMEPAD_BUTTON_UP — button={}", event.gbutton.button);
            break;

        // ------------------------------------------------------------------
        // 手柄摇杆轴（含死区过滤）
        // ------------------------------------------------------------------
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        {
            m_input.activeDevice = InputDeviceType::Gamepad;
            const auto  axis  = event.gaxis.axis;
            float       value = event.gaxis.value / 32767.0f;

            if (std::abs(value) < 0.2f) value = 0.0f;

            m_logger->debug("[Event] SDL_EVENT_GAMEPAD_AXIS_MOTION — axis={} raw={} normalized={:.3f}",
                axis, event.gaxis.value, value);

            if (axis == SDL_GAMEPAD_AXIS_LEFTX) rawAxisX = value;
            if (axis == SDL_GAMEPAD_AXIS_LEFTY) rawAxisY = value;
            break;
        }

        default:
            break;
        }
    }

    // ------------------------------------------------------------------
    // 统一处理持续移动状态（融合键盘与手柄，键盘优先覆盖摇杆）
    // ------------------------------------------------------------------
    const bool* keys = SDL_GetKeyboardState(nullptr);
    m_input.moveX = rawAxisX;
    m_input.moveY = rawAxisY;

    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])  m_input.moveX = -1.0f;
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) m_input.moveX =  1.0f;
    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])    m_input.moveY = -1.0f;
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])  m_input.moveY =  1.0f;

    // 归一化斜向移动，避免对角速度变为 √2 倍
    const float length = std::sqrt(m_input.moveX * m_input.moveX + m_input.moveY * m_input.moveY);
    if (length > 1.0f)
    {
        m_input.moveX /= length;
        m_input.moveY /= length;
    }
}

} // namespace lw

