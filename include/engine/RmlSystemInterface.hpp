#ifndef LOOMWOOD_RML_SYSTEM_INTERFACE_HPP
#define LOOMWOOD_RML_SYSTEM_INTERFACE_HPP

#include <RmlUi/Core/SystemInterface.h>
#include <spdlog/spdlog.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace lw
{

// System bridge for RmlUi.
// Provides elapsed time, log forwarding, and a dictionary-based
// localization hook via TranslateString().
// The inner dictionary (locale → key → text) is backed by
// std::unordered_map for now; replace with JSON loading later.
class RmlSystemInterface final : public Rml::SystemInterface
{
public:
    explicit RmlSystemInterface();

    // Switch active locale and populate the dictionary.
    void setLocale(const std::string& locale);

    [[nodiscard]] const std::string& locale() const noexcept { return m_locale; }

    // --- Rml::SystemInterface ---
    double GetElapsedTime() override;

    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;

    // Intercepts every text node in .rml; looks up the string as a key
    // in the active locale dictionary. Returns 1 on hit, 0 on miss
    // (RmlUi renders the raw key as fallback).
    int TranslateString(Rml::String& translated, const Rml::String& input) override;

private:
    void loadBuiltinDictionaries();

    std::string m_locale = "en_US";

    // [locale][key] → translated text
    std::unordered_map<std::string,
        std::unordered_map<std::string, std::string>> m_dictionaries;

    std::shared_ptr<spdlog::logger> m_logger;
};

} // namespace lw

#endif // LOOMWOOD_RML_SYSTEM_INTERFACE_HPP
