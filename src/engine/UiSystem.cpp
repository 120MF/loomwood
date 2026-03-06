#include <engine/UiSystem.hpp>
#include <engine/WindowService.hpp>
#include <log/log.hpp>

#include <RmlUi/Core.h>
#include <entt/entt.hpp>

#include <filesystem>

namespace lw
{

UiSystem::UiSystem()
{
    m_logger = create_logger("UiSystem");

    m_renderInterface = std::make_unique<RmlRenderInterface>();
    m_systemInterface = std::make_unique<RmlSystemInterface>();

    Rml::SetRenderInterface(m_renderInterface.get());
    Rml::SetSystemInterface(m_systemInterface.get());

    if (!Rml::Initialise())
        log_fatal_and_throw(m_logger, "Rml::Initialise() failed");

    m_logger->info("RmlUi initialised");
}

UiSystem::~UiSystem()
{
    if (m_context)
    {
        Rml::RemoveContext(m_context->GetName());
        m_context = nullptr;
    }
    Rml::Shutdown();
    m_logger->info("RmlUi shutdown");
}

void UiSystem::init(const std::string& locale)
{
    m_systemInterface->setLocale(locale);

    loadFonts();

    // RmlUi 上下文始终使用逻辑（设计）分辨率，
    // SDL_SetRenderLogicalPresentation 负责将渲染结果缩放到物理窗口。
    auto& window = entt::locator<WindowService>::value();
    const int w = window.designWidth();
    const int h = window.designHeight();

    m_context = Rml::CreateContext("main", Rml::Vector2i(w, h));
    if (!m_context)
        log_fatal_and_throw(m_logger, "Rml::CreateContext() failed");

    // 初始化 dp-ratio，让 RmlUi 按物理像素密度栅格化字体。
    setDpRatio(SDL_GetWindowDisplayScale(window.window()));
    m_logger->info("UI context '{}' created ({}x{}, dp-ratio={:.2f})",
        "main", w, h, SDL_GetWindowDisplayScale(window.window()));

    // --- 加载测试菜单（后续由场景管理器按需加载）---
    auto* doc = m_context->LoadDocument("assets/ui/main_menu.rml");
    if (doc)
    {
        doc->Show();
        m_logger->info("Loaded document: main_menu.rml");
    }
    else
    {
        m_logger->warn("Failed to load document: main_menu.rml");
    }
}

void UiSystem::loadFonts()
{
    const std::filesystem::path font_dir = "assets/fonts";
    if (!std::filesystem::exists(font_dir))
    {
        m_logger->warn("Font directory '{}' not found", font_dir.string());
        return;
    }

    int loaded = 0;
    for (const auto& entry : std::filesystem::directory_iterator(font_dir))
    {
        const auto& p = entry.path();
        if (p.extension() == ".ttf" || p.extension() == ".otf")
        {
            if (Rml::LoadFontFace(p.string()))
            {
                m_logger->info("Loaded font: {}", p.filename().string());
                ++loaded;
            }
            else
            {
                m_logger->warn("Failed to load font: {}", p.string());
            }
        }
    }
    m_logger->info("{} font face(s) loaded", loaded);
}

void UiSystem::setDpRatio(float ratio)
{
    if (m_context)
        m_context->SetDensityIndependentPixelRatio(ratio);
}

void UiSystem::update(float /*dt*/)
{
    if (m_context)
        m_context->Update();
}

void UiSystem::render()
{
    if (m_context)
        m_context->Render();
}

} // namespace lw
