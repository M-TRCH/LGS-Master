
#include "system.h"

DebugLevel debugLevel = DEBUG_VERBOSE;    // Set debug level (none-basic-verbose)

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

    set_relay(true);
    PRINT(DEBUG_BASIC, F("Waiting for Module Startup\n"));
    uint32_t startupTime = millis();
    while(millis() - startupTime < MODULE_STARTUP_DELAY)
    {
        PRINT(DEBUG_BASIC, F("."));
        delay(100);
    }
    PRINT(DEBUG_BASIC, F("\nSystem Initialized\n"));
}

void set_relay(bool state)
{
    digitalWrite(RELAY_2_PIN, state);
    digitalWrite(RELAY_4_PIN, state);
}

bool debounce_sw(uint32_t pin, uint32_t debounceTime, uint32_t releaseTimeout)
{
    if (digitalRead(pin) == HIGH)
    {
        delay(debounceTime);
        if (digitalRead(pin) == HIGH)
        {
            uint32_t startTime = millis();
            while (digitalRead(pin) == HIGH)
            {
                if (millis() - startTime > releaseTimeout)
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}
