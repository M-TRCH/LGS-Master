
#ifndef SYSTEM_H
#define SYSTEM_H
#include <Arduino.h>
#include <mbed.h>
#include "config.h"
#include "logger.h"

// global variables
extern uint32_t kick_watchdog_timer;
extern bool local_mode_active;

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
bool debounce_sw(uint32_t pin, uint32_t debounceTime, uint32_t releaseTimeout);

/* @brief Print switch states for debugging
 */
void panel_switch_debug();

#endif