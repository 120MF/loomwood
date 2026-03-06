#ifndef LOOMWOOD_UI_SYSTEM_HPP
#define LOOMWOOD_UI_SYSTEM_HPP

#include <engine/RmlRenderInterface.hpp>
#include <engine/RmlSystemInterface.hpp>

#include <RmlUi/Core/Context.h>
#include <spdlog/spdlog.h>
#include <memory>
#include <string>

namespace lw
{

// Manages the full RmlUi lifecycle: interface installation, context
// creation, font loading, and per-frame update / render.
//
// Lifecycle:
//   UiSystem()        – install interfaces, Rml::Initialise()
//   init(locale)      – set locale, create context, load fonts
//   update(dt) / render() – called every frame
//   ~UiSystem()       – Rml::Shutdown()
class UiSystem
{
public:
    UiSystem();
    ~UiSystem();

    // Call once in Engine::run() before the main loop.
    void init(const std::string& locale);

    // Update dp-ratio (call on init and on SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED).
    void setDpRatio(float ratio);

    // Per-frame: advance RmlUi time and resolve animations.
    void update(float dt);

    // Per-frame: submit geometry via the RenderInterface.
    void render();

    [[nodiscard]] Rml::Context* context() const noexcept { return m_context; }
    [[nodiscard]] RmlSystemInterface& systemInterface() noexcept { return *m_systemInterface; }

private:
    void loadFonts();

    std::unique_ptr<RmlRenderInterface> m_renderInterface;
    std::unique_ptr<RmlSystemInterface> m_systemInterface;

    Rml::Context* m_context = nullptr;
    std::shared_ptr<spdlog::logger> m_logger;
};

} // namespace lw

#endif // LOOMWOOD_UI_SYSTEM_HPP
