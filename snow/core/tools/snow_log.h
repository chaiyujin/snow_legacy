#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace snow {

/* get logger of spdlog */
void            InitLogger(const char *name);
spdlog::logger *Logger();

/* easy access */
template<typename... Args> void info(const char *fmt, const Args &... args)  { Logger()->info(fmt, args...);                }
template<typename... Args> void warn(const char *fmt, const Args &... args)  { Logger()->warn(fmt, args...);                }
template<typename... Args> void debug(const char *fmt, const Args &... args) { Logger()->debug(fmt, args...);               }
template<typename... Args> void error(const char *fmt, const Args &... args) { Logger()->error(fmt, args...);               }
template<typename... Args> void fatal(const char *fmt, const Args &... args) { Logger()->critical(fmt, args...); exit(1);   }
/* assert */
template<typename... Args> void assertion(bool flag) {
    if (!flag) { Logger()->critical("assertion failed"); exit(1); }
}
template<typename... Args> void assertion(bool flag, const char *fmt, const Args &... args) {
    if (!flag) { Logger()->critical(fmt, args...); exit(1); }
}

}