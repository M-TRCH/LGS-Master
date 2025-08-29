
#include "system.h"

void system_init(void)
{
    // pins configuration
    pinMode(RELAY_1_PIN, OUTPUT);
    pinMode(RELAY_2_PIN, OUTPUT);
    pinMode(RELAY_3_PIN, OUTPUT);
    pinMode(RELAY_4_PIN, OUTPUT);
    pinMode(W_SW_PIN, INPUT_PULLUP);
    pinMode(R_SW_PIN, INPUT_PULLUP);
    pinMode(G_SW_PIN, INPUT_PULLUP);
    pinMode(B_SW_PIN, INPUT_PULLUP);
    pinMode(Y_SW_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN_0_PIN, OUTPUT);
    pinMode(LED_BUILTIN_1_PIN, OUTPUT);
    pinMode(LED_BUILTIN_2_PIN, OUTPUT);
    pinMode(LED_BUILTIN_3_PIN, OUTPUT);

    // serial configuration
    Serial.begin(SERIAL_BAUD);
    Serial.setTimeout(SERIAL_TIMEOUT);
    
    Serial.print(F("Waiting for Serial"));
    uint32_t startTime = millis();
    while(!Serial && millis() - startTime < 2000)
    {
        Serial.print(F("."));
        delay(200);
    }
    Serial.println(F("System Initialized"));  
}