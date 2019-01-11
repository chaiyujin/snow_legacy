#include "snow_log.h"

namespace snow {

static std::shared_ptr<spdlog::logger> gConsoleLogger(nullptr);

void _init(const char *name) {
    gConsoleLogger = spdlog::stdout_color_mt(name);
#ifdef NDEBUG
    spdlog::set_level(spdlog::level::info);
#else
    spdlog::set_level(spdlog::level::debug);
#endif
}

void InitLogger(const char *name) {
    if (gConsoleLogger == nullptr) _init(name);
    else                           throw std::runtime_error("[snow]: logger is inited.");
}
spdlog::logger *Logger() {
    if (gConsoleLogger == nullptr)  _init("snow");
    return                          gConsoleLogger.get();
}

}