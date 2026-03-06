#include <engine/Engine.hpp>
#include <iostream>
#include <exception>

#include "log/fatal.hpp"
using namespace lw;

int main()
{
  // 设置 std::terminate 处理器，捕获未预料异常
  std::set_terminate([]()
  {
    try
    {
      // 尝试获取当前异常信息（如果有）
      if (const std::exception_ptr eptr = std::current_exception())
      {
        std::rethrow_exception(eptr);
      }
    }
    catch (const std::exception& e)
    {
      fatal(std::format("Unhandled exception：{}", e.what()));
    }
    catch (...)
    {
      fatal("Unhandled exception：Unknown");
    }
    // 如果上面没有抛出，手动调用 fatal
    fatal("std::terminate is called");
  });

  try
  {
    Engine engine;
    engine.run();
  }
  catch (const std::exception& e)
  {
    fatal(std::format("Exception：{}", e.what()));
  }
  catch (...)
  {
    fatal("Unknow Exception");
  }
  return 0;
}
