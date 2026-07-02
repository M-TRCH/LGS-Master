
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Module type enumeration
enum class ModuleType 
{
    STANDARD,   // Standard module (row 1-8)
    NARCOTIC    // Narcotic module (row 0-9)
};

// Struct for storing IP address
struct IPAddress_t 
{
    uint8_t ip1;
    uint8_t ip2;
    uint8_t ip3;
    uint8_t ip4;
};

// Struct for storing firmware version
struct FirmwareVersion_t 
{
    uint8_t day;    // day (1-31)
    uint8_t month;  // month (1-12)
    uint16_t year;  // year (e.g. 2025)
};

// Struct for storing all device information
struct DeviceInfo_t 
{
    IPAddress_t ip_address;
    FirmwareVersion_t firmware_version;
};

namespace ProjectConfig
{
    struct RgbColor
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    namespace Device
    {
        static constexpr ModuleType MODULE_TYPE = ModuleType::STANDARD;
        static constexpr IPAddress_t IP_ADDRESS = {192, 168, 0, 12};
        static constexpr FirmwareVersion_t FIRMWARE_VERSION = {2, 7, 2026};
    }

    namespace Pins
    {
        static constexpr pin_size_t RELAY_1 = D0;
        static constexpr pin_size_t RELAY_2 = D1;
        static constexpr pin_size_t RELAY_3 = D2;
        static constexpr pin_size_t RELAY_4 = D3;
        static constexpr pin_size_t SWITCH_WHITE = A0;
        static constexpr pin_size_t SWITCH_RED = A1;
        static constexpr pin_size_t SWITCH_GREEN = A2;
        static constexpr pin_size_t SWITCH_BLUE = A3;
        static constexpr pin_size_t SWITCH_YELLOW = A4;
        static constexpr pin_size_t LED_0 = LED_D0;
        static constexpr pin_size_t LED_1 = LED_D1;
        static constexpr pin_size_t LED_2 = LED_D2;
        static constexpr pin_size_t LED_3 = LED_D3;
    }

    namespace System
    {
        static constexpr uint32_t SERIAL_BAUD = 9600;
        static constexpr uint32_t SERIAL_TIMEOUT_MS = 100;
        static constexpr uint32_t RS485_BAUD = 9600;
        static constexpr uint32_t RS485_TIMEOUT_MS = 50;
        static constexpr uint32_t MODULE_STARTUP_DELAY_MS = 5000;
        static constexpr uint32_t STARTUP_MODE_POLL_INTERVAL_MS = 100;
        static constexpr uint32_t WATCHDOG_TIMEOUT_MS = 30000;
        static constexpr uint32_t WATCHDOG_FEED_INTERVAL_MS = WATCHDOG_TIMEOUT_MS / 4;
        static constexpr uint32_t SWITCH_DEBOUNCE_MS = 50;
        static constexpr uint32_t SWITCH_RELEASE_TIMEOUT_MS = 1000;
    }

    namespace Network
    {
        static constexpr uint16_t TCP_SERVER_PORT = 2000;
        static constexpr uint32_t CLIENT_TIMEOUT_MS = 120000;
        static constexpr size_t MAX_DEVICE_COUNT = 9999;
        static constexpr IPAddress_t GATEWAY = {192, 168, 0, 1};
        static constexpr IPAddress_t SUBNET = {255, 255, 255, 0};
        static constexpr IPAddress_t DNS = {8, 8, 8, 8};
        static constexpr bool MQTT_ENABLED = false;
        static constexpr const char* MQTT_BROKER_IP = "192.168.0.60";
        static constexpr uint16_t MQTT_BROKER_PORT = 1883;
        static constexpr const char* MQTT_USERNAME = "admin";
        static constexpr const char* MQTT_PASSWORD = "admin";
        static constexpr uint32_t MQTT_RECONNECT_INTERVAL_MS = 15UL * 60000UL;
        static constexpr const char* MQTT_CLIENT_PREFIX = "lgs";
        static constexpr const char* MQTT_TOPIC_PREFIX = "lgs/";
        static constexpr const char* MQTT_TOPIC_SUFFIX = "/opta";
    }

    namespace Protocol
    {
        static constexpr char PACKET_HEADER = 'B';
        static constexpr uint8_t PACKET_FIELD_COUNT = 10;
        static constexpr uint8_t CHECKSUM_MODULO = 100;
        static constexpr uint8_t MODULE_ID_MULTIPLIER = 10;
        static constexpr uint8_t STANDARD_ROW_OFFSET = 9;
        static constexpr uint8_t NARCOTIC_ROW_OFFSET = 10;
        static constexpr uint8_t CABINET_WIDTH = 2;
        static constexpr uint8_t ROW_WIDTH = 2;
        static constexpr uint8_t COLUMN_WIDTH = 2;
        static constexpr uint8_t QUANTITY_WIDTH = 4;
        static constexpr uint8_t COLOR_WIDTH = 2;
        static constexpr uint8_t COMMAND_WIDTH = 2;
        static constexpr uint8_t STATUS_WIDTH = 2;
        static constexpr uint8_t TRANSITION_WIDTH = 2;
        static constexpr uint8_t DEVICE_WIDTH = 4;
        static constexpr uint8_t CHECKSUM_WIDTH = 2;
        static constexpr int MAX_TWO_DIGIT_VALUE = 99;
        static constexpr int MAX_FOUR_DIGIT_VALUE = 9999;
        static constexpr int MAX_DEVICE_ID = static_cast<int>(Network::MAX_DEVICE_COUNT) - 1;
        static constexpr int COLOR_RED = 1;
        static constexpr int COLOR_GREEN = 2;
        static constexpr int COLOR_BLUE = 3;
        static constexpr int COLOR_YELLOW = 4;
    }

    namespace Colors
    {
        static constexpr RgbColor CLEAR = {0, 0, 0};
        static constexpr RgbColor RED = {255, 0, 0};
        static constexpr RgbColor GREEN = {0, 255, 0};
        static constexpr RgbColor BLUE = {0, 0, 255};
        static constexpr RgbColor YELLOW = {255, 145, 0};
        static constexpr RgbColor WHITE = {255, 255, 255};
        static constexpr uint8_t PANEL_CODE_CLEAR = 0;
        static constexpr uint8_t PANEL_CODE_RED = 1;
        static constexpr uint8_t PANEL_CODE_GREEN = 2;
        static constexpr uint8_t PANEL_CODE_BLUE = 3;
        static constexpr uint8_t PANEL_CODE_YELLOW = 4;
        static constexpr uint8_t PANEL_CODE_WHITE = 5;
    }

    namespace Lgs
    {
        static constexpr uint8_t PANEL_DISPLAY_ID = 99;
        static constexpr uint8_t PANEL_WRITE_LENGTH = 5;
        static constexpr uint8_t PANEL_SEND_TIMEOUT_MS = 50;
        static constexpr uint8_t PANEL_SEND_RETRIES = 2;
        static constexpr uint8_t STANDARD_WRITE_LENGTH = 3;
        static constexpr uint8_t STANDARD_SEND_TIMEOUT_MS = 50;
        static constexpr uint8_t STANDARD_SEND_RETRIES = 3;
        static constexpr uint8_t NARCOTIC_ON_WRITE_LENGTH = 3;
        static constexpr uint8_t NARCOTIC_OFF_WRITE_LENGTH = 1;
        static constexpr uint8_t NARCOTIC_SEND_TIMEOUT_MS = 200;
        static constexpr uint8_t NARCOTIC_SEND_RETRIES = 3;
        static constexpr uint8_t STATUS_SEND_TIMEOUT_MS = 200;
        static constexpr uint8_t STATUS_SEND_RETRIES = 3;
        static constexpr uint8_t YEAR_DISPLAY_MODULO = 100;
        static constexpr uint8_t STANDARD_MIN_ROW = 1;
        static constexpr uint8_t STANDARD_MAX_ROW = 8;
        static constexpr uint8_t STANDARD_MIN_COLUMN = 1;
        static constexpr uint8_t STANDARD_MAX_COLUMN = 8;
        static constexpr uint8_t STANDARD_MIN_COLOR = 1;
        static constexpr uint8_t STANDARD_MAX_COLOR = 4;
        static constexpr uint8_t NARCOTIC_MIN_ROW = 0;
        static constexpr uint8_t NARCOTIC_MAX_ROW = 9;
        static constexpr uint8_t NARCOTIC_MIN_COLUMN = 1;
        static constexpr uint8_t NARCOTIC_MAX_COLUMN = 8;
        static constexpr uint8_t NARCOTIC_REQUIRED_COLOR = Protocol::COLOR_RED;
        static constexpr float STANDARD_BRIGHTNESS = 1.0f;
        static constexpr float NARCOTIC_ON_BRIGHTNESS = 0.8f;
        static constexpr float NARCOTIC_TEST_BRIGHTNESS = 0.2f;
        static constexpr float OFF_BRIGHTNESS = 0.0f;
        static constexpr uint8_t COLOR_SCALE = 255;
        static constexpr uint16_t LOCAL_MODE_PREVIEW_DELAY_MS = 800;
        static constexpr uint16_t RESET_INDICATOR_HOLD_MS = 2000;
        static constexpr uint16_t RESET_CLEAR_HOLD_MS = 500;
        static constexpr uint16_t RELAY_SHUTDOWN_HOLD_MS = 500;
        static constexpr uint16_t LOOP_YIELD_DELAY_MS = 0;
    }
}

extern ModuleType device_type;

/* @brief Initialize device configuration from centralized project settings.
 */
void config_init();

// Global variable to store device information
extern DeviceInfo_t device_info;

/* @brief Set the IP address of the device
 * @param info Pointer to the DeviceInfo_t structure
 * @param ip1 First octet of the IP address
 * @param ip2 Second octet of the IP address
 * @param ip3 Third octet of the IP address
 * @param ip4 Fourth octet of the IP address
 */
void setIPAddress(DeviceInfo_t* info, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4);

/*
 * @brief Set the firmware version of the device
 * @param info Pointer to the DeviceInfo_t structure
 * @param day Day of the firmware version (1-31)
 * @param month Month of the firmware version (1-12)
 * @param year Year of the firmware version (e.g. 2025)
 */
void setFirmwareVersion(DeviceInfo_t* info, uint8_t day, uint8_t month, uint16_t year);

/* @brief Convert IP address to String
 * @param ip Pointer to the IPAddress_t structure
 */
String ipToString(const IPAddress_t* ip);

/* @brief Convert firmware version to String
 * @param version Pointer to the FirmwareVersion_t structure
 */
String firmwareVersionToString(const FirmwareVersion_t* version);

/* @brief Print device information
 * @param info Pointer to the DeviceInfo_t structure
 */
void printDeviceInfo(const DeviceInfo_t* info);

#endif // CONFIG_H
