#include "log.h"

namespace snow { namespace log {

static bool gLevelSet = false;
static std::vector<std::shared_ptr<spdlog::logger>> gLoggers;

std::vector<std::shared_ptr<spdlog::logger>> &get_loggers() {
    if (!gLevelSet) {
        spdlog::set_level(__RAY_LOG_LEVEL__);
        gLevelSet = true;
    }
    return gLoggers;
}

}}