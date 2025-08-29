
#ifndef SYSTEM_H
#define SYSTEM_H
#include <Arduino.h>

// pins configuration
#define RELAY_1_PIN         D0
#define RELAY_2_PIN         D1
#define RELAY_3_PIN         D2
#define RELAY_4_PIN         D3
#define W_SW_PIN            A0
#define R_SW_PIN            A1
#define G_SW_PIN            A2
#define B_SW_PIN            A3
#define Y_SW_PIN            A4
#define LED_BUILTIN_0_PIN   LED_D0
#define LED_BUILTIN_1_PIN   LED_D1
#define LED_BUILTIN_2_PIN   LED_D2
#define LED_BUILTIN_3_PIN   LED_D3

// serial configuration
#define SERIAL_BAUD         9600
#define SERIAL_TIMEOUT      100
#define RS485_BAUD          9600
#define RS485_TIMEOUT       50

void system_init(void);

#endif