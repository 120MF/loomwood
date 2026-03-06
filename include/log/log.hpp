#ifndef LOOMWOOD_LOG_HPP
#define LOOMWOOD_LOG_HPP

#include <spdlog/spdlog.h>

namespace lw {

using sink_ptr = spdlog::sink_ptr;
void init_logging();
bool is_logging_initialized() noexcept;
std::shared_ptr<spdlog::logger>
create_logger(const std::string &name,
              spdlog::level::level_enum level = spdlog::level::info);
std::shared_ptr<spdlog::logger> get_default_logger();

} // namespace lw

#endif // LOOMWOOD_LOG_HPP
