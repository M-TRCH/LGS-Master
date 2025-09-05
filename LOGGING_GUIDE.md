# LGS Master - Logging System

## Overview
A new flexible and easy-to-manage logging system that supports message categorization and configurable output levels.

## Features
- **Multiple Log Levels**: ERROR, WARNING, INFO, DEBUG, VERBOSE
- **Categorization**: SYSTEM, NETWORK, TCP, MQTT, LGS, CONFIG
- **Configurable**: Enable/disable individual categories and levels
- **Standardized Format**: Consistent log message formatting
- **Backward Compatible**: Supports legacy PRINT() macro

## Log Levels
```cpp
enum LogLevel {
    LOG_NONE = 0,     // No output
    LOG_ERROR,        // Critical errors
    LOG_WARNING,      // Warnings
    LOG_INFO,         // General information
    LOG_DEBUG,        // Debug information
    LOG_VERBOSE       // Detailed information
};
```

## Categories
```cpp
enum LogCategory {
    CAT_SYSTEM = 0,   // General system
    CAT_NETWORK,      // Ethernet networking
    CAT_TCP,          // TCP Server/Client
    CAT_MQTT,         // MQTT Communication
    CAT_LGS,          // LGS Module Operations
    CAT_CONFIG        // Configuration
};
```

## Usage Examples

### Basic Logging
```cpp
LOG_INFO_MSG(CAT_SYSTEM, "System started");
LOG_ERROR_MSG(CAT_NETWORK, "Network connection failed");
LOG_DEBUG_MSG(CAT_TCP, "Packet received");
```

### Printf-style Logging
```cpp
LOG_INFO_F(CAT_TCP, "Client connected from %s:%d", ip, port);
LOG_DEBUG_F(CAT_LGS, "Module [%d,%d] status: %d", row, col, status);
LOG_ERROR_F(CAT_MQTT, "Failed to publish to topic %s", topic);
```

### Configuration
```cpp
// Set global log level
logger_set_level(LOG_DEBUG);

// Enable/disable categories
logger_set_category(CAT_VERBOSE, false);  // Disable verbose logs
logger_set_category(CAT_MQTT, true);      // Enable MQTT logs
```

## Output Format
```
[timestamp] [LEVEL] [CATEGORY] message
```

Example:
```
[1234] [INFO] [SYSTEM] Logger initialized
[1567] [ERROR] [NETWORK] Ethernet cable not connected
[2890] [DEBUG] [TCP] Packet received - Cabinet:1 Row:8 Col:1
```

## MQTT Integration
The system supports sending messages via MQTT in JSON format:

```cpp
// Send different message types
mqtt_publish_json(MqttMessageType::INFO, "System started");
mqtt_publish_json(MqttMessageType::WARNING, "Low battery");
mqtt_publish_json(MqttMessageType::ERROR, "Connection lost");
```

JSON output format:
```json
{
  "type": "info",
  "message": "System started"
}
```

## Migration from Old System
Legacy system (PRINT macro) continues to work:
```cpp
PRINT(DEBUG_BASIC, "Old style message");   // Converted to LOG_INFO
PRINT(DEBUG_VERBOSE, "Detailed message"); // Converted to LOG_VERBOSE
```

## Best Practices
1. Use appropriate levels: ERROR for errors, INFO for general information
2. Choose correct categories: network-related messages use CAT_NETWORK
3. Use printf-style for messages with variables
4. Set log level appropriately: production uses LOG_INFO, development uses LOG_DEBUG
