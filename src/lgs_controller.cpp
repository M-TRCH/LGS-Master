#include "lgs_controller.h"

namespace
{
    ModuleColor to_module_color(const ProjectConfig::RgbColor& color)
    {
        return ModuleColor(color.r, color.g, color.b);
    }

    bool color_matches(const ModuleColor& color, const ProjectConfig::RgbColor& config_color)
    {
        return color.r == config_color.r && color.g == config_color.g && color.b == config_color.b;
    }

    uint8_t to_panel_color_code(const ModuleColor& color)
    {
        if (color_matches(color, ProjectConfig::Colors::RED)) return ProjectConfig::Colors::PANEL_CODE_RED;
        if (color_matches(color, ProjectConfig::Colors::GREEN)) return ProjectConfig::Colors::PANEL_CODE_GREEN;
        if (color_matches(color, ProjectConfig::Colors::BLUE)) return ProjectConfig::Colors::PANEL_CODE_BLUE;
        if (color_matches(color, ProjectConfig::Colors::YELLOW)) return ProjectConfig::Colors::PANEL_CODE_YELLOW;
        if (color_matches(color, ProjectConfig::Colors::WHITE)) return ProjectConfig::Colors::PANEL_CODE_WHITE;
        return ProjectConfig::Colors::PANEL_CODE_CLEAR;
    }

    uint8_t to_standard_command_address(const ModuleColor& color)
    {
        if (color_matches(color, ProjectConfig::Colors::RED)) return LGSAddress::LGS_LED12;
        if (color_matches(color, ProjectConfig::Colors::GREEN)) return LGSAddress::LGS_LED34;
        if (color_matches(color, ProjectConfig::Colors::BLUE)) return LGSAddress::LGS_LED56;
        if (color_matches(color, ProjectConfig::Colors::YELLOW)) return LGSAddress::LGS_LED78;
        return ProjectConfig::Colors::PANEL_CODE_CLEAR;
    }

    uint8_t to_narcotic_command_color(const ModuleColor& color)
    {
        if (color_matches(color, ProjectConfig::Colors::RED)) return LGSAddress::LGS_LED1;
        if (color_matches(color, ProjectConfig::Colors::GREEN)) return LGSAddress::LGS_LED2;
        if (color_matches(color, ProjectConfig::Colors::BLUE)) return LGSAddress::LGS_LED3;
        if (color_matches(color, ProjectConfig::Colors::YELLOW)) return LGSAddress::LGS_LED4;
        return ProjectConfig::Colors::PANEL_CODE_CLEAR;
    }
}

LGSbus lgs;
ModuleColor cl_clear = to_module_color(ProjectConfig::Colors::CLEAR);
ModuleColor cl_red = to_module_color(ProjectConfig::Colors::RED);
ModuleColor cl_green = to_module_color(ProjectConfig::Colors::GREEN);
ModuleColor cl_blue = to_module_color(ProjectConfig::Colors::BLUE);
ModuleColor cl_yellow = to_module_color(ProjectConfig::Colors::YELLOW);
ModuleColor cl_white = to_module_color(ProjectConfig::Colors::WHITE);
ModuleStatus_t DEFAULT_STATUS = ModuleStatus_t::MODULE_UNKNOWN;

void lgs_init()
{
    // Initialize RS485 communication for LGS bus
    lgs.begin(lgs.ID_MASTER, &RS485, ProjectConfig::System::RS485_BAUD, ProjectConfig::System::RS485_TIMEOUT_MS);
    RS485.receive();    

    // In local mode, cycle through colors on the panel display.
    if (local_mode_active)
    {
        set_info(cl_red, device_info);      delay(ProjectConfig::Lgs::LOCAL_MODE_PREVIEW_DELAY_MS);
        set_info(cl_green, device_info);    delay(ProjectConfig::Lgs::LOCAL_MODE_PREVIEW_DELAY_MS);
        set_info(cl_blue, device_info);     delay(ProjectConfig::Lgs::LOCAL_MODE_PREVIEW_DELAY_MS);
        set_info(cl_yellow, device_info);   delay(ProjectConfig::Lgs::LOCAL_MODE_PREVIEW_DELAY_MS);
        set_info(cl_white, device_info);    delay(ProjectConfig::Lgs::LOCAL_MODE_PREVIEW_DELAY_MS);
        set_info(cl_clear, device_info);  
    }   
}

bool set_info(const ModuleColor& color, const DeviceInfo_t& info)
{
    uint8_t color_code = to_panel_color_code(color);

    // Send device information to the panel display
    lgs.writeData(LGSAddress::LGS_GREET, 0, color_code);
    lgs.writeData(LGSAddress::LGS_GREET, 1, ProjectConfig::Colors::PANEL_CODE_CLEAR); // Unused
    lgs.writeData(LGSAddress::LGS_GREET, 2, device_info.firmware_version.day);
    lgs.writeData(LGSAddress::LGS_GREET, 3, device_info.firmware_version.month);
    lgs.writeData(LGSAddress::LGS_GREET, 4, device_info.firmware_version.year % ProjectConfig::Lgs::YEAR_DISPLAY_MODULO);
    return lgs.write(ProjectConfig::Lgs::PANEL_DISPLAY_ID, ProjectConfig::Lgs::PANEL_WRITE_LENGTH, LGSAddress::LGS_GREET, ProjectConfig::Lgs::PANEL_SEND_TIMEOUT_MS, ProjectConfig::Lgs::PANEL_SEND_RETRIES);
}

bool set_color(const ModuleType& type, const ModuleAddress& addr, const ModuleColor& color, float brightness, bool state, int quantity, ModuleStatus_t& status)
{
    // Validate brightness range
    if (brightness < 0.0f) brightness = 0.0f;
    if (brightness > 1.0f) brightness = 1.0f;

    // Get module id
    int module_id = addr.get_id();
    if (module_id < 0) 
    {
        LOG_ERROR_MSG(CAT_LGS, "Invalid module address");
        return false;
    }
    
    // Send command based on module type
    if (type == ModuleType::STANDARD) 
    {
        // Command parameters
        uint8_t cmd_addr = to_standard_command_address(color);

        // Calculate RGB values
        const int r = state ? int(color.r * brightness) : 0;
        const int g = state ? int(color.g * brightness) : 0;
        const int b = state ? int(color.b * brightness) : 0;

        // Mapping color to command address (light location)
        if (cmd_addr == ProjectConfig::Colors::PANEL_CODE_CLEAR)
        {
            LOG_ERROR_MSG(CAT_LGS, "Invalid color for STANDARD module");
            return false;
        }

        // Send color command
        lgs.writeData(cmd_addr, 0, r);
        lgs.writeData(cmd_addr, 1, g);
        lgs.writeData(cmd_addr, 2, b);
        return lgs.write(module_id, ProjectConfig::Lgs::STANDARD_WRITE_LENGTH, cmd_addr, ProjectConfig::Lgs::STANDARD_SEND_TIMEOUT_MS, ProjectConfig::Lgs::STANDARD_SEND_RETRIES);
    }
    else if (type == ModuleType::NARCOTIC)
    {
        uint8_t cmd_color = to_narcotic_command_color(color);
        
        if (cmd_color == ProjectConfig::Colors::PANEL_CODE_CLEAR)
        {
            LOG_ERROR_MSG(CAT_LGS, "Invalid color for NARCOTIC module");
            return false;
        }

        // Send turn on command
        if (state)
        {
            lgs.writeData(LGSAddress::LGS_LED1, 0, cmd_color);
            lgs.writeData(LGSAddress::LGS_LED1, 1, brightness * ProjectConfig::Lgs::COLOR_SCALE);
            lgs.writeData(LGSAddress::LGS_LED1, 2, abs(quantity));
            return lgs.write(module_id, ProjectConfig::Lgs::NARCOTIC_ON_WRITE_LENGTH, LGSAddress::LGS_LED1, ProjectConfig::Lgs::NARCOTIC_SEND_TIMEOUT_MS, ProjectConfig::Lgs::NARCOTIC_SEND_RETRIES);
        }
        // Send turn off command
        else
        {
            // reset data buffer before sending OFF command
            lgs.writeData(LGSAddress::LGS_GREET, 0, 0);
            lgs.writeData(LGSAddress::LGS_LED1, 0, 0);

            // read sensor data before sending OFF command
            if (!lgs.read(module_id, LGSAddress::LGS_GREET, ProjectConfig::Lgs::NARCOTIC_SEND_TIMEOUT_MS, ProjectConfig::Lgs::NARCOTIC_SEND_RETRIES))
            {
                LOG_ERROR_F(CAT_LGS, "Failed to read before sending OFF command to module ID %d", module_id);
                return false;
            }
            else
            {
                // If the sensor data is true. (in position)
                if (lgs.readData(LGSAddress::LGS_GREET, 0))
                {
                    status = ModuleStatus_t::MODULE_IDLE;
                    return lgs.write(module_id, ProjectConfig::Lgs::NARCOTIC_OFF_WRITE_LENGTH, LGSAddress::LGS_LED1, ProjectConfig::Lgs::NARCOTIC_SEND_TIMEOUT_MS, ProjectConfig::Lgs::NARCOTIC_SEND_RETRIES);
                }
                // If the sensor data is false. (not in position)
                else
                {
                    LOG_ERROR_F(CAT_LGS, "Module ID %d not in position, cannot turn off", module_id);
                    status = ModuleStatus_t::MODULE_BUSY;
                    return false;
                }
            }
        }
    }
    return true;
}

bool request_status(const ModuleType& type, const ModuleAddress& addr, const ModuleColor& color, ModuleStatus_t& status)
{
    // Get module id
    int module_id = addr.get_id();
    if (module_id < 0) 
    {
        LOG_ERROR_MSG(CAT_LGS, "Invalid module address");
        return false;
    }
    
    // Send command based on module type
    if (type == ModuleType::STANDARD) 
    {
        // Command parameters
        uint8_t cmd_addr = to_standard_command_address(color);

        if (cmd_addr == ProjectConfig::Colors::PANEL_CODE_CLEAR)
        {
            LOG_ERROR_MSG(CAT_LGS, "Invalid color for STANDARD module");
            return false;
        }

        if (lgs.read(module_id, cmd_addr, ProjectConfig::Lgs::STATUS_SEND_TIMEOUT_MS, ProjectConfig::Lgs::STATUS_SEND_RETRIES))
        {
            int r = lgs.readData(cmd_addr, 0);
            int g = lgs.readData(cmd_addr, 1);
            int b = lgs.readData(cmd_addr, 2);
            if (r == 0 && g == 0 && b == 0) status = ModuleStatus_t::MODULE_IDLE;
            else                            status = ModuleStatus_t::MODULE_BUSY;
            return true;
        }
        else
        {
            status = ModuleStatus_t::MODULE_ERROR;
            LOG_ERROR_F(CAT_LGS, "Failed to read status from module ID %d", module_id);
            return false;
        }
    }
    else if (type == ModuleType::NARCOTIC)
    {
        // Command parameters
        uint8_t cmd_addr = to_narcotic_command_color(color);

        if (cmd_addr == ProjectConfig::Colors::PANEL_CODE_CLEAR)
        {
            LOG_ERROR_MSG(CAT_LGS, "Invalid color for NARCOTIC module");
            return false;
        }

        if (lgs.read(module_id, cmd_addr, ProjectConfig::Lgs::STATUS_SEND_TIMEOUT_MS, ProjectConfig::Lgs::STATUS_SEND_RETRIES))
        {
            int sensor = lgs.readData(cmd_addr, 0);
            if (sensor == 0)    status = ModuleStatus_t::MODULE_IDLE;   // In position
            else                status = ModuleStatus_t::MODULE_BUSY;   // Not in position
            return true;
        }
        else
        {
            status = ModuleStatus_t::MODULE_ERROR;
            LOG_ERROR_F(CAT_LGS, "Failed to read status from module ID %d", module_id);
            return false;
        }   
    }
    return true;
}

void soft_reset(bool indicate)
{
    if (indicate)
    {
        // Indicate reset with white LED
        set_info(cl_white, device_info);
        delay(ProjectConfig::Lgs::RESET_INDICATOR_HOLD_MS);
        set_info(cl_clear, device_info);
        delay(ProjectConfig::Lgs::RESET_CLEAR_HOLD_MS);
    }
    set_relay(false);  // Turn off relay
    delay(ProjectConfig::Lgs::RELAY_SHUTDOWN_HOLD_MS);
    NVIC_SystemReset();
}

void white_button_event()
{
    if (debounce_sw(ProjectConfig::Pins::SWITCH_WHITE, ProjectConfig::System::SWITCH_DEBOUNCE_MS, ProjectConfig::System::SWITCH_RELEASE_TIMEOUT_MS))
    {
        soft_reset();   // Perform soft reset with LED indication
    }
}

void red_button_event()
{
    if (debounce_sw(ProjectConfig::Pins::SWITCH_RED, ProjectConfig::System::SWITCH_DEBOUNCE_MS, ProjectConfig::System::SWITCH_RELEASE_TIMEOUT_MS))
    {
        set_info(cl_red, device_info);  // Indicate config mode with red LED

        if (device_type == ModuleType::STANDARD)
        {
            for (uint8_t row = ProjectConfig::Lgs::STANDARD_MIN_ROW; row <= ProjectConfig::Lgs::STANDARD_MAX_ROW; row++)
            {
                mbed::Watchdog::get_instance().kick();

                for (uint8_t col = ProjectConfig::Lgs::STANDARD_MIN_COLUMN; col <= ProjectConfig::Lgs::STANDARD_MAX_COLUMN; col++)
                {
                    bool red_success = set_color(ModuleType::STANDARD, ModuleAddress(row, col), cl_red, ProjectConfig::Lgs::STANDARD_BRIGHTNESS, true);
                    bool green_success = set_color(ModuleType::STANDARD, ModuleAddress(row, col), cl_green, ProjectConfig::Lgs::STANDARD_BRIGHTNESS, true);
                    bool blue_success = set_color(ModuleType::STANDARD, ModuleAddress(row, col), cl_blue, ProjectConfig::Lgs::STANDARD_BRIGHTNESS, true);
                    bool yellow_success = set_color(ModuleType::STANDARD, ModuleAddress(row, col), cl_yellow, ProjectConfig::Lgs::STANDARD_BRIGHTNESS, true);
                    LOG_DEBUG_F(CAT_LGS, "Set color at [%d, %d]: R=%s, G=%s, B=%s, Y=%s", 
                        row, col, 
                        red_success ? "Success" : "Fail", 
                        green_success ? "Success" : "Fail", 
                        blue_success ? "Success" : "Fail", 
                        yellow_success ? "Success" : "Fail");
                }
            }
        }
        else if (device_type == ModuleType::NARCOTIC)
        {
            // Placeholder for narcotic module handling
        }

        set_info(cl_clear, device_info); // Clear LED after config
    }    
}

void green_button_event()
{
    if (debounce_sw(ProjectConfig::Pins::SWITCH_GREEN, ProjectConfig::System::SWITCH_DEBOUNCE_MS, ProjectConfig::System::SWITCH_RELEASE_TIMEOUT_MS))
    {   
        set_info(cl_green, device_info);  // Indicate config mode with green LED

        if (device_type == ModuleType::STANDARD)
        {
            for (uint8_t row = ProjectConfig::Lgs::STANDARD_MIN_ROW; row <= ProjectConfig::Lgs::STANDARD_MAX_ROW; row++)
            {
                mbed::Watchdog::get_instance().kick();
                
                for (uint8_t col = ProjectConfig::Lgs::STANDARD_MIN_COLUMN; col <= ProjectConfig::Lgs::STANDARD_MAX_COLUMN; col++)
                {
                    bool red_success = set_color(ModuleType::STANDARD, ModuleAddress(row, col), cl_red);
                    bool green_success = set_color(ModuleType::STANDARD, ModuleAddress(row, col), cl_green);
                    bool blue_success = set_color(ModuleType::STANDARD, ModuleAddress(row, col), cl_blue);
                    bool yellow_success = set_color(ModuleType::STANDARD, ModuleAddress(row, col), cl_yellow);
                    LOG_DEBUG_F(CAT_LGS, "Set color at [%d, %d]: R=%s, G=%s, B=%s, Y=%s", 
                        row, col, 
                        red_success ? "Success" : "Fail", 
                        green_success ? "Success" : "Fail", 
                        blue_success ? "Success" : "Fail", 
                        yellow_success ? "Success" : "Fail");
                }
            }
        }
        else if (device_type == ModuleType::NARCOTIC)
        {
            // Placeholder for narcotic module handling
        }

        set_info(cl_clear, device_info); // Clear LED after config
    }
}

void blue_button_event()
{
    if (debounce_sw(ProjectConfig::Pins::SWITCH_BLUE, ProjectConfig::System::SWITCH_DEBOUNCE_MS, ProjectConfig::System::SWITCH_RELEASE_TIMEOUT_MS))
    {
        set_info(cl_blue, device_info);  // Indicate config mode with blue LED

        if (device_type == ModuleType::STANDARD)
        {
            // Placeholder for standard module handling
        }

        else if (device_type == ModuleType::NARCOTIC)
        {
            for (uint8_t col = ProjectConfig::Lgs::NARCOTIC_MIN_COLUMN; col <= ProjectConfig::Lgs::NARCOTIC_MAX_COLUMN; col++)
            {
                mbed::Watchdog::get_instance().kick();

                for (uint8_t row = ProjectConfig::Lgs::NARCOTIC_MIN_ROW; row <= ProjectConfig::Lgs::NARCOTIC_MAX_ROW; row++)
                {
                    bool success = set_color(ModuleType::NARCOTIC, ModuleAddress(row, col), cl_red, ProjectConfig::Lgs::NARCOTIC_TEST_BRIGHTNESS, true, (row * ProjectConfig::Protocol::MODULE_ID_MULTIPLIER + col));
                    LOG_DEBUG_F(CAT_LGS, "Set color at [%d, %d]: %s", 
                        row, col, 
                        success ? "Success" : "Fail");
                        
                    delay(ProjectConfig::Lgs::LOOP_YIELD_DELAY_MS);
                }
            }
        }

        set_info(cl_clear, device_info); // Clear LED after config
    }
}

void yellow_button_event()
{
    if (debounce_sw(ProjectConfig::Pins::SWITCH_YELLOW, ProjectConfig::System::SWITCH_DEBOUNCE_MS, ProjectConfig::System::SWITCH_RELEASE_TIMEOUT_MS))
    {
        set_info(cl_yellow, device_info);  // Indicate config mode with yellow LED

        if (device_type == ModuleType::STANDARD)
        {
            // Placeholder for standard module handling
        }

        else if (device_type == ModuleType::NARCOTIC)
        {
             for (uint8_t col = ProjectConfig::Lgs::NARCOTIC_MIN_COLUMN; col <= ProjectConfig::Lgs::NARCOTIC_MAX_COLUMN; col++)
            {
                mbed::Watchdog::get_instance().kick();

                for (uint8_t row = ProjectConfig::Lgs::NARCOTIC_MIN_ROW; row <= ProjectConfig::Lgs::NARCOTIC_MAX_ROW; row++)
                {
                    ModuleStatus_t status = ModuleStatus_t::MODULE_UNKNOWN;
                    bool success = set_color(ModuleType::NARCOTIC, ModuleAddress(row, col), cl_red, ProjectConfig::Lgs::OFF_BRIGHTNESS, false, (row * ProjectConfig::Protocol::MODULE_ID_MULTIPLIER + col), status);
                    LOG_DEBUG_F(CAT_LGS, "Set OFF at [%d, %d]: %s, Status=%d", 
                        row, col, 
                        success ? "Success" : "Fail",
                        status);                            
                    delay(ProjectConfig::Lgs::LOOP_YIELD_DELAY_MS);
                }
            }
        }

        set_info(cl_clear, device_info); // Clear LED after config
    }
}
