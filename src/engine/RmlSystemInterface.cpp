#include <engine/RmlSystemInterface.hpp>
#include <log/log.hpp>

#include <SDL3/SDL.h>

namespace lw
{

namespace
{
// Returns true and writes 'out' if 'key' is found in the given locale bucket.
bool lookupKey(
    const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& dicts,
    const std::string& locale,
    const std::string& key,
    Rml::String&       out)
{
    const auto locale_it = dicts.find(locale);
    if (locale_it == dicts.end())
        return false;
    const auto key_it = locale_it->second.find(key);
    if (key_it == locale_it->second.end())
        return false;
    out = key_it->second;
    return true;
}
} // namespace

RmlSystemInterface::RmlSystemInterface()
{
    m_logger = create_logger("RmlUi");
    loadBuiltinDictionaries();
}

void RmlSystemInterface::setLocale(const std::string& locale)
{
    m_locale = locale;
    m_logger->info("UI locale set to '{}'", locale);
}

// ---------------------------------------------------------------------------
// Built-in string table
// Replace this block with JSON / binary loading once the pipeline is ready.
// ---------------------------------------------------------------------------
void RmlSystemInterface::loadBuiltinDictionaries()
{
    auto& zh = m_dictionaries["zh_CN"];
    zh["ui_main_start_game"]  = "开始游戏";
    zh["ui_main_continue"]    = "继续";
    zh["ui_main_settings"]    = "设置";
    zh["ui_main_quit"]        = "退出";
    zh["ui_settings_title"]   = "设置";
    zh["ui_settings_volume"]  = "音量";
    zh["ui_settings_confirm"] = "确认";
    zh["ui_settings_back"]    = "返回";

    auto& en = m_dictionaries["en_US"];
    en["ui_main_start_game"]  = "Start Game";
    en["ui_main_continue"]    = "Continue";
    en["ui_main_settings"]    = "Settings";
    en["ui_main_quit"]        = "Quit";
    en["ui_settings_title"]   = "Settings";
    en["ui_settings_volume"]  = "Volume";
    en["ui_settings_confirm"] = "Confirm";
    en["ui_settings_back"]    = "Back";
}

// ---------------------------------------------------------------------------
// Rml::SystemInterface
// ---------------------------------------------------------------------------

double RmlSystemInterface::GetElapsedTime()
{
    return static_cast<double>(SDL_GetTicksNS()) / 1'000'000'000.0;
}

bool RmlSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
    switch (type)
    {
        case Rml::Log::LT_ERROR:   m_logger->error("{}", message);   break;
        case Rml::Log::LT_WARNING: m_logger->warn("{}",  message);   break;
        case Rml::Log::LT_INFO:    m_logger->info("{}",  message);   break;
        default:                   m_logger->debug("{}", message);    break;
    }
    return true; // returning true keeps RmlUi's own assertion handler quiet
}

int RmlSystemInterface::TranslateString(
    Rml::String&       translated,
    const Rml::String& input)
{
    if (lookupKey(m_dictionaries, m_locale, input, translated))
        return 1;
    // Fallback: try en_US before giving up.
    if (m_locale != "en_US" && lookupKey(m_dictionaries, "en_US", input, translated))
        return 1;
    translated = input;
    return 0;
}

} // namespace lw
