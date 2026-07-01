
#include "system.h"

uint32_t kick_watchdog_timer = 0;
bool local_mode_active = true;

void system_init(void)
{
    // pins configuration
    pinMode(ProjectConfig::Pins::RELAY_1, OUTPUT);
    pinMode(ProjectConfig::Pins::RELAY_2, OUTPUT);
    pinMode(ProjectConfig::Pins::RELAY_3, OUTPUT);
    pinMode(ProjectConfig::Pins::RELAY_4, OUTPUT);
    pinMode(ProjectConfig::Pins::SWITCH_WHITE, INPUT_PULLUP);
    pinMode(ProjectConfig::Pins::SWITCH_RED, INPUT_PULLUP);
    pinMode(ProjectConfig::Pins::SWITCH_GREEN, INPUT_PULLUP);
    pinMode(ProjectConfig::Pins::SWITCH_BLUE, INPUT_PULLUP);
    pinMode(ProjectConfig::Pins::SWITCH_YELLOW, INPUT_PULLUP);
    pinMode(ProjectConfig::Pins::LED_0, OUTPUT);
    pinMode(ProjectConfig::Pins::LED_1, OUTPUT);
    pinMode(ProjectConfig::Pins::LED_2, OUTPUT);
    pinMode(ProjectConfig::Pins::LED_3, OUTPUT);
    
    // serial configuration
    Serial.begin(ProjectConfig::System::SERIAL_BAUD);
    Serial.setTimeout(ProjectConfig::System::SERIAL_TIMEOUT_MS);

    set_relay(true);
    LOG_INFO_MSG(CAT_SYSTEM, "Waiting for Module Startup");
    uint32_t startupTime = millis();
    while(millis() - startupTime < ProjectConfig::System::MODULE_STARTUP_DELAY_MS)
    {
        // Holding all four colored buttons during startup switches to network mode.
        if (!digitalRead(ProjectConfig::Pins::SWITCH_RED) &&
            !digitalRead(ProjectConfig::Pins::SWITCH_GREEN) &&
            !digitalRead(ProjectConfig::Pins::SWITCH_BLUE) &&
            !digitalRead(ProjectConfig::Pins::SWITCH_YELLOW))
        {
            local_mode_active = false;
        }
        delay(ProjectConfig::System::STARTUP_MODE_POLL_INTERVAL_MS);
    }
    LOG_INFO_MSG(CAT_SYSTEM, "Module Startup Complete");
}

void set_relay(bool state)
{
    digitalWrite(ProjectConfig::Pins::RELAY_2, state);
    digitalWrite(ProjectConfig::Pins::RELAY_4, state);
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
    Serial.print(digitalRead(ProjectConfig::Pins::SWITCH_WHITE));
    Serial.print("\tR:");
    Serial.print(digitalRead(ProjectConfig::Pins::SWITCH_RED));
    Serial.print("\tG:");
    Serial.print(digitalRead(ProjectConfig::Pins::SWITCH_GREEN));
    Serial.print("\tB:");
    Serial.print(digitalRead(ProjectConfig::Pins::SWITCH_BLUE));
    Serial.print("\tY:");
    Serial.print(digitalRead(ProjectConfig::Pins::SWITCH_YELLOW));
    Serial.println();
}
