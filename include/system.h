
#ifndef SYSTEM_H
#define SYSTEM_H
#include <Arduino.h>

// pins configuration
#define RELAY_1_PIN             D0
#define RELAY_2_PIN             D1
#define RELAY_3_PIN             D2
#define RELAY_4_PIN             D3
#define W_SW_PIN                A0
#define R_SW_PIN                A1
#define G_SW_PIN                A2
#define B_SW_PIN                A3
#define Y_SW_PIN                A4
#define LED_BUILTIN_0_PIN       LED_D0
#define LED_BUILTIN_1_PIN       LED_D1
#define LED_BUILTIN_2_PIN       LED_D2
#define LED_BUILTIN_3_PIN       LED_D3

// constants values
#define SERIAL_BAUD             9600
#define SERIAL_TIMEOUT          100
#define RS485_BAUD              9600
#define RS485_TIMEOUT           50
#define MODULE_STARTUP_DELAY    5000

enum DebugLevel
{
    DEBUG_NONE = 0,
    DEBUG_BASIC,
    DEBUG_VERBOSE
};
extern DebugLevel debugLevel;

// macro definitions
#define PRINT(level, msg) \
    do { if (debugLevel >= level) Serial.print(msg); } while(0)

/* @brief Initialize system: pins, serial communication, and config
 */
void system_init(void);

/* @brief Set relay state
 * @param state: true to turn on, false to turn off
 */
void set_relay(bool state=true);

/*
 * @brief Debounce switch input
 * @param pin: pin number
 * @param delayTime: debounce delay time in milliseconds
 * @return true if switch is pressed, false otherwise
 */
bool debounce_sw(uint32_t pin, uint32_t debounceTime=50, uint32_t releaseTimeout=1000);

/* @brief Print switch states for debugging
 */
void panel_switch_debug();

#endif