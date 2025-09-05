#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

// Log levels
enum LogLevel {
    LOG_NONE = 0,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
    LOG_VERBOSE
};

// Log categories
enum LogCategory {
    CAT_SYSTEM = 0,
    CAT_NETWORK,
    CAT_TCP,
    CAT_MQTT,
    CAT_LGS,
    CAT_CONFIG,
    CAT_COUNT  // Must be last - used for array size
};

// Global log configuration
extern LogLevel g_log_level;
extern bool g_category_enabled[CAT_COUNT];

// Category names for display
extern const char* CATEGORY_NAMES[CAT_COUNT];

/**
 * @brief Initialize logging system
 * @param level Global log level
 */
void logger_init(LogLevel level = LOG_INFO);

/**
 * @brief Enable/disable specific category
 * @param category Category to control
 * @param enabled true to enable, false to disable
 */
void logger_set_category(LogCategory category, bool enabled);

/**
 * @brief Set global log level
 * @param level New log level
 */
void logger_set_level(LogLevel level);

/**
 * @brief Main logging function
 * @param level Log level of this message
 * @param category Log category
 * @param message Message to log
 */
void log_message(LogLevel level, LogCategory category, const String& message);

/**
 * @brief Log with format string support
 */
void log_printf(LogLevel level, LogCategory category, const char* format, ...);

// Convenience macros for each level
#define LOG_ERROR_MSG(cat, msg)   log_message(LOG_ERROR, cat, msg)
#define LOG_WARN_MSG(cat, msg)    log_message(LOG_WARNING, cat, msg)
#define LOG_INFO_MSG(cat, msg)    log_message(LOG_INFO, cat, msg)
#define LOG_DEBUG_MSG(cat, msg)   log_message(LOG_DEBUG, cat, msg)
#define LOG_VERBOSE_MSG(cat, msg) log_message(LOG_VERBOSE, cat, msg)

// Convenience macros for printf-style formatting
#define LOG_ERROR_F(cat, fmt, ...)   log_printf(LOG_ERROR, cat, fmt, ##__VA_ARGS__)
#define LOG_WARN_F(cat, fmt, ...)    log_printf(LOG_WARNING, cat, fmt, ##__VA_ARGS__)
#define LOG_INFO_F(cat, fmt, ...)    log_printf(LOG_INFO, cat, fmt, ##__VA_ARGS__)
#define LOG_DEBUG_F(cat, fmt, ...)   log_printf(LOG_DEBUG, cat, fmt, ##__VA_ARGS__)
#define LOG_VERBOSE_F(cat, fmt, ...) log_printf(LOG_VERBOSE, cat, fmt, ##__VA_ARGS__)

// Legacy PRINT macro replacement - maps to new system
#define PRINT(level, msg) \
    do { \
        LogLevel new_level = (level == DEBUG_BASIC) ? LOG_INFO : \
                            (level == DEBUG_VERBOSE) ? LOG_VERBOSE : LOG_NONE; \
        if (new_level != LOG_NONE) log_message(new_level, CAT_SYSTEM, String(msg)); \
    } while(0)

#endif // LOGGER_H
