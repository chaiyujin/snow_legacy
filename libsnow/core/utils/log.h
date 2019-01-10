#pragma once
#ifdef NDEBUG
// release
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#define __RAY_LOG_LEVEL__ spdlog::level::info
#else
// debug
#define SPDLOG_TRACE_ON
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#define __RAY_LOG_LEVEL__ spdlog::level::trace
#endif

namespace snow { namespace log {

/* get loggers of spdlog and set level */
std::vector<std::shared_ptr<spdlog::logger>> &GetLoggers();

/* set logger */
inline void AddLogger(const char *name)     { 
    GetLoggers().push_back(std::move(spdlog::stdout_color_mt(name)));
}
inline void AddFileLogger(const char *name, const char *path) {
    GetLoggers().push_back(std::move(spdlog::rotating_logger_mt(name, path, 1048576 * 5, 3)));
}

/* auto init */
inline void __Check() {
    auto &logs = GetLoggers();
    if (logs.size() == 0) { 
        AddLogger("snow");
        AddFileLogger("snow-file", "snow.log");
    }
}

/* easy access */
template<typename... Args> inline void info(const char *fmt, const Args &... args)  { __Check(); for (auto logger : GetLoggers()) logger->info(fmt, args...); }
template<typename... Args> inline void warn(const char *fmt, const Args &... args)  { __Check(); for (auto logger : GetLoggers()) logger->warn(fmt, args...); }
template<typename... Args> inline void error(const char *fmt, const Args &... args) { __Check(); for (auto logger : GetLoggers()) logger->error(fmt, args...); }
template<typename... Args> inline void fatal(const char *fmt, const Args &... args) { __Check(); for (auto logger : GetLoggers()) logger->critical(fmt, args...); exit(1); }
#ifdef NDEBUG
template<typename... Args> inline void debug(const char *fmt, const Args &... args) {}
template<typename... Args> inline void assertion(bool flag, const char *fmt, const Args &... args) {}
template<typename... Args> inline void assertion(bool flag) {}
#else
template<typename... Args> inline void debug(const char *fmt, const Args &... args) { __Check(); for (auto logger : GetLoggers()) logger->debug(fmt, args...); }
template<typename... Args> inline void assertion(bool flag, const char *fmt, const Args &... args) {
    if (!flag) {
        __Check();
        for (auto logger : GetLoggers())
            logger->critical(fmt, args...);
        exit(1);
    }
}
template<typename... Args> inline void assertion(bool flag) {
    if (!flag) {
        __Check();
        for (auto logger : GetLoggers())
            logger->critical("assertion failed");
        exit(1);
    }
}
#endif

}}