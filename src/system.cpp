
#include "system.h"

uint32_t kick_watchdog_timer = 0;
bool dev_mode_activated = true;

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
    LOG_INFO_MSG(CAT_SYSTEM, "Waiting for Module Startup");
    uint32_t startupTime = millis();
    while(millis() - startupTime < MODULE_STARTUP_DELAY)
    {
        // Check for developer mode activation
        if (!digitalRead(R_SW_PIN) && !digitalRead(G_SW_PIN) && !digitalRead(B_SW_PIN) && !digitalRead(Y_SW_PIN))
        {
            dev_mode_activated = false;
        }
        delay(100);
    }
    LOG_INFO_MSG(CAT_SYSTEM, "Module Startup Complete");
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

void panel_switch_debug()
{
    Serial.print("W:");
    Serial.print(digitalRead(W_SW_PIN));
    Serial.print("\tR:");
    Serial.print(digitalRead(R_SW_PIN));
    Serial.print("\tG:");
    Serial.print(digitalRead(G_SW_PIN));
    Serial.print("\tB:");
    Serial.print(digitalRead(B_SW_PIN));
    Serial.print("\tY:");
    Serial.print(digitalRead(Y_SW_PIN));
    Serial.println();
}
