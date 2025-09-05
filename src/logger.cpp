#include "logger.h"
#include <stdarg.h>

// Global variables
LogLevel g_log_level = LOG_INFO;
bool g_category_enabled[CAT_COUNT] = {true, true, true, true, true, true};

// Category names for display
const char* CATEGORY_NAMES[CAT_COUNT] = {
    "SYSTEM",
    "NETWORK", 
    "TCP",
    "MQTT",
    "LGS",
    "CONFIG"
};

// Level names for display
const char* LEVEL_NAMES[] = {
    "",
    "ERROR",
    "WARN",
    "INFO", 
    "DEBUG",
    "VERBOSE"
};

void logger_init(LogLevel level) {
    g_log_level = level;
    
    // Enable all categories by default
    for (int i = 0; i < CAT_COUNT; i++) {
        g_category_enabled[i] = true;
    }
    
    Serial.begin(9600);
    log_message(LOG_INFO, CAT_SYSTEM, "Logger initialized");
}

void logger_set_category(LogCategory category, bool enabled) {
    if (category >= 0 && category < CAT_COUNT) {
        g_category_enabled[category] = enabled;
    }
}

void logger_set_level(LogLevel level) {
    g_log_level = level;
    log_message(LOG_INFO, CAT_SYSTEM, "Log level changed to " + String(LEVEL_NAMES[level]));
}

void log_message(LogLevel level, LogCategory category, const String& message) {
    // Check if logging is enabled for this level and category
    if (level > g_log_level || !g_category_enabled[category]) {
        return;
    }
    
    // Get current timestamp (millis since start)
    unsigned long timestamp = millis();
    
    // Format: [timestamp] [LEVEL] [CATEGORY] message
    Serial.print("[");
    Serial.print(timestamp);
    Serial.print("] [");
    Serial.print(LEVEL_NAMES[level]);
    Serial.print("] [");
    Serial.print(CATEGORY_NAMES[category]);
    Serial.print("] ");
    Serial.println(message);
}

void log_printf(LogLevel level, LogCategory category, const char* format, ...) {
    // Check if logging is enabled for this level and category
    if (level > g_log_level || !g_category_enabled[category]) {
        return;
    }
    
    // Format the message using vsnprintf
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Use the main log_message function
    log_message(level, category, String(buffer));
}
