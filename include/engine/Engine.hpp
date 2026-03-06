#ifndef LOOMWOOD_ENGINE_HPP
#define LOOMWOOD_ENGINE_HPP

#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

namespace lw {

class Engine {
public:
  Engine(std::string_view title, int width, int height);
  ~Engine();
  void run();

private:
  void initGameObjects();
  void processInput();
  void update(float dt);
  void render();

  SDL_Window *m_window = nullptr;
  SDL_Renderer *m_renderer = nullptr;
  bool m_isRunning = false;
  uint64_t m_lastTime = 0;
  entt::registry m_registry;
  std::shared_ptr<spdlog::logger> m_logger;
};

} // namespace lw

#endif // LOOMWOOD_ENGINE_HPP
