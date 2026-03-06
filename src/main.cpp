#include <engine/Engine.hpp>
#include <iostream>
using namespace lw;
int main() {
  try {
    Engine engine("织源工坊 - Engine Core Test", 800, 600);
    engine.run();
  } catch (const std::exception &e) {
    std::cerr << "Engine Fatal Error: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}