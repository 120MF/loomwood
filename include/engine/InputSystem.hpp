#ifndef LOOMWOOD_INPUT_SYSTEM_HPP
#define LOOMWOOD_INPUT_SYSTEM_HPP

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <memory>
#include <unordered_map>

namespace lw
{

enum class InputDeviceType
{
    KeyboardMouse,
    Gamepad
};

struct VirtualInput
{
    // 移动轴 (范围 -1.0 到 1.0)
    float moveX = 0.0f;
    float moveY = 0.0f;

    // UI 导航信号（区分"按下瞬间"与"按住"）
    bool uiUpJustPressed      = false;
    bool uiDownJustPressed    = false;
    bool uiConfirmJustPressed = false;
    bool uiCancelJustPressed  = false;

    // 当前激活设备，用于切换 UI 提示图标
    InputDeviceType activeDevice = InputDeviceType::KeyboardMouse;

    // 每帧开始前清理 JustPressed 状态
    void resetTriggers()
    {
        uiUpJustPressed      = false;
        uiDownJustPressed    = false;
        uiConfirmJustPressed = false;
        uiCancelJustPressed  = false;
    }
};

class InputSystem
{
public:
    InputSystem();
    ~InputSystem();

    void process(bool& isRunning);

    [[nodiscard]] const VirtualInput& input() const noexcept { return m_input; }

private:
    VirtualInput m_input;
    std::unordered_map<SDL_JoystickID, SDL_Gamepad*> m_gamepads;
    std::shared_ptr<spdlog::logger> m_logger;
};

} // namespace lw

#endif // LOOMWOOD_INPUT_SYSTEM_HPP
