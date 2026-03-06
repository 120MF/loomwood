#ifndef LOOMWOOD_ENGINE_HPP
#define LOOMWOOD_ENGINE_HPP

#include <entt/entt.hpp>
#include <spdlog/spdlog.h>
#include <string_view>
#include <memory>

namespace lw {

class Engine {
public:
    Engine();
    ~Engine();
    void run();

private:
    void initGameObjects();

    bool m_isRunning = false;
    uint64_t m_lastTime = 0;
    entt::registry m_registry;
    std::shared_ptr<spdlog::logger> m_logger;
};

} // namespace lw

#endif // LOOMWOOD_ENGINE_HPP
