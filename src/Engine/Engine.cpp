#include <Engine/Engine.hpp>

struct TransformComponent {
  float x, y;
  float width, height;
};

struct VelocityComponent {
  float dx, dy;
};

struct ColorComponent {
  uint8_t r, g, b, a;
};

namespace lw {
Engine::Engine(std::string_view title, int width, int height) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    throw std::runtime_error("SDL_Init 失败: " + std::string(SDL_GetError()));
  }

  m_window =
      SDL_CreateWindow(title.data(), width, height, SDL_WINDOW_RESIZABLE);
  if (!m_window) {
    throw std::runtime_error("创建窗口失败: " + std::string(SDL_GetError()));
  }
  m_renderer = SDL_CreateRenderer(m_window, nullptr);
  if (!m_renderer) {
    throw std::runtime_error("创建渲染器失败: " + std::string(SDL_GetError()));
  }
  SDL_SetRenderVSync(m_renderer, 1);
  m_isRunning = true;
  m_lastTime = SDL_GetTicksNS();
  initGameObjects();
}
Engine::~Engine() {
  if (m_renderer)
    SDL_DestroyRenderer(m_renderer);
  if (m_window)
    SDL_DestroyWindow(m_window);
  SDL_Quit();
}
void Engine::run() {
  while (m_isRunning) {
    processInput();

    uint64_t current_time = SDL_GetTicksNS();
    float dt = static_cast<float>(current_time - m_lastTime) / 1000000000.0f;
    m_lastTime = current_time;
    update(dt);
    render();
  }
}
void Engine::initGameObjects() {
  auto dragon = m_registry.create();
  m_registry.emplace<TransformComponent>(dragon, 100.0f, 100.0f, 50.0f, 50.0f);
  m_registry.emplace<VelocityComponent>(dragon, 150.0f, 100.0f); // 每秒移动像素
  m_registry.emplace<ColorComponent>(dragon, 255, 255, 255, 255); // 白色

  // 创建一个小镇 NPC
  auto npc = m_registry.create();
  m_registry.emplace<TransformComponent>(npc, 400.0f, 300.0f, 40.0f, 40.0f);
  // NPC 没有 Velocity，所以是个站桩实体
  m_registry.emplace<ColorComponent>(npc, 100, 200, 100, 255); // 浅绿色
}
void Engine::processInput() {
  SDL_Event event;
  // 把操作系统事件队列里的事件全部抽干
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      m_isRunning = false; // 点击窗口的 X 按钮
    }
    if (event.type == SDL_EVENT_KEY_DOWN) {
      if (event.key.key == SDLK_ESCAPE) {
        m_isRunning = false; // 按 ESC 退出
      }
    }
  }
}
void Engine::update(float dt) {
  // 物理系统：查找所有同时拥有 Transform 和 Velocity 的 Entity
  auto view = m_registry.view<TransformComponent, VelocityComponent>();

  // C++17 的结构化绑定，极其优雅的遍历方式
  for (auto entity : view) {
    auto &transform = view.get<TransformComponent>(entity);
    auto &velocity = view.get<VelocityComponent>(entity);

    // 更新位置 (根据增量时间)
    transform.x += velocity.dx * dt;
    transform.y += velocity.dy * dt;

    // 简单的边界反弹逻辑 (模拟撞墙)
    if (transform.x <= 0 || transform.x + transform.width >= 800) {
      velocity.dx *= -1;
    }
    if (transform.y <= 0 || transform.y + transform.height >= 600) {
      velocity.dy *= -1;
    }
  }
}
void Engine::render() {
  SDL_SetRenderDrawColor(m_renderer, 30, 30, 30, 255);
  SDL_RenderClear(m_renderer);

  auto view = m_registry.view<TransformComponent, ColorComponent>();
  for (auto entity : view) {
    auto &transform = view.get<TransformComponent>(entity);
    auto &color = view.get<ColorComponent>(entity);

    // 设置 Entity专属的颜色
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

    // 组装硬件加速 API 需要的浮点矩形结构 (SDL_FRect 是 SDL3 的标准)
    SDL_FRect rect = {transform.x, transform.y, transform.width,
                      transform.height};

    // 提交绘制指令 (FillRect)
    SDL_RenderFillRect(m_renderer, &rect);
  }

  // 步骤 D: 交换缓冲区，推送到屏幕上 (Present)
  SDL_RenderPresent(m_renderer);
}
} // namespace lw