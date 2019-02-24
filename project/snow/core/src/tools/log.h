#pragma once
#include "../common.h"
#ifdef NDEBUG // release
#define __RAY_LOG_LEVEL__ spdlog::level::info
#else // debug
#define SPDLOG_TRACE_ON
#define __RAY_LOG_LEVEL__ spdlog::level::trace
#endif
#include "../../third-party/spdlog/spdlog.h"
#include "../../third-party/spdlog/fmt/ostr.h"
#include "../../third-party/spdlog/sinks/stdout_color_sinks.h"
#include "../../third-party/spdlog/sinks/rotating_file_sink.h"


namespace snow { namespace log {

/* get loggers of spdlog and set level */
SNOW_API std::vector<std::shared_ptr<spdlog::logger>> &get_loggers();

// add logger
SNOW_INLINE void add_logger(const char *name)     { 
    get_loggers().push_back(spdlog::stdout_color_mt(name));
}
// add file logger
SNOW_INLINE void add_logger(const char *name, const char *path) {
    get_loggers().push_back(spdlog::rotating_logger_mt(name, path, 1048576 * 5, 3));
}

/* auto init */
SNOW_INLINE void __Check() {
    auto &logs = get_loggers();
    if (logs.size() == 0) { 
        add_logger("snow");
        add_logger("snow-file", "snow.log");
    }
}

/* easy access */
template<typename... Args> SNOW_INLINE void info(const char *fmt, const Args &... args)  { __Check(); for (auto logger : get_loggers()) logger->info(fmt, args...); }
template<typename... Args> SNOW_INLINE void warn(const char *fmt, const Args &... args)  { __Check(); for (auto logger : get_loggers()) logger->warn(fmt, args...); }
template<typename... Args> SNOW_INLINE void error(const char *fmt, const Args &... args) { __Check(); for (auto logger : get_loggers()) logger->error(fmt, args...); }
template<typename... Args> SNOW_INLINE void fatal(const char *fmt, const Args &... args) { __Check(); for (auto logger : get_loggers()) logger->critical(fmt, args...); exit(1); }
#ifdef NDEBUG
template<typename... Args> SNOW_INLINE void debug(const char *fmt, const Args &... args) {}
template<typename... Args> SNOW_INLINE void assertion(bool flag, const char *fmt, const Args &... args) {}
template<typename... Args> SNOW_INLINE void assertion(bool flag) {}
#else
template<typename... Args> SNOW_INLINE void debug(const char *fmt, const Args &... args) { __Check(); for (auto logger : get_loggers()) logger->debug(fmt, args...); }
template<typename... Args> SNOW_INLINE void assertion(bool flag, const char *fmt, const Args &... args) {
    if (!flag) {
        __Check();
        for (auto logger : get_loggers())
            logger->critical(fmt, args...);
        exit(1);
    }
}
template<typename... Args> SNOW_INLINE void assertion(bool flag) {
    if (!flag) {
        __Check();
        for (auto logger : get_loggers())
            logger->critical("assertion failed");
        exit(1);
    }
}
#endif

}}