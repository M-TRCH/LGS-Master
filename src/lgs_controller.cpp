#include "lgs_controller.h"

LGSbus lgs;
ModuleColor cl_clear(0, 0, 0);
ModuleColor cl_red(255, 0, 0);  
ModuleColor cl_green(0, 255, 0);
ModuleColor cl_blue(0, 0, 255);
ModuleColor cl_yellow(255, 145, 0);
ModuleColor cl_white(255, 255, 255);

void lgs_init()
{
  lgs.begin(lgs.ID_MASTER, &RS485, RS485_BAUD, RS485_TIMEOUT);
  RS485.receive();
}

bool set_info(const ModuleColor& color, const DeviceInfo_t& info)
{
    const uint8_t cmd_id = 99;              // Set 99 as the panel display ID
    const uint8_t cmd_addr_num = 5;         // Number of command address to write
    const uint8_t cmd_send_timeout = 50;    // Timeout for command send
    const uint8_t cmd_send_retries = 2;     // Number of retries for command send
    uint8_t color_code = 0;                 // Color code for LED indicator
    
    if (color.r == 255 && color.g == 0 && color.b == 0)         color_code = 1; // Red
    else if (color.r == 0 && color.g == 255 && color.b == 0)    color_code = 2; // Green
    else if (color.r == 0 && color.g == 0 && color.b == 255)    color_code = 3; // Blue
    else if (color.r == 255 && color.g == 145 && color.b == 0)  color_code = 4; // Yellow
    else if (color.r == 255 && color.g == 255 && color.b == 255)color_code = 5; // White
    else                                                        color_code = 0; // Off/Unknown

    // Send device information to the panel display
    lgs.writeData(LGSAddress::GREET, 0, color_code);
    lgs.writeData(LGSAddress::GREET, 1, 0); // Unused   
    lgs.writeData(LGSAddress::GREET, 2, device_info.firmware_version.day);
    lgs.writeData(LGSAddress::GREET, 3, device_info.firmware_version.month);
    lgs.writeData(LGSAddress::GREET, 4, device_info.firmware_version.year % 100); // Last two digits of year
    return lgs.write(cmd_id, cmd_addr_num, LGSAddress::GREET, cmd_send_timeout, cmd_send_retries);
}

bool set_color(const ModuleType& type, const ModuleAddress& addr, const ModuleColor& color, float brightness, bool state)
{
    // Validate brightness range
    if (brightness < 0.0f) brightness = 0.0f;
    if (brightness > 1.0f) brightness = 1.0f;

    // Calculate RGB values
    int r = state ? int(color.r * brightness) : 0;
    int g = state ? int(color.g * brightness) : 0;
    int b = state ? int(color.b * brightness) : 0;

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
        const uint8_t cmd_addr_num = 3;         // Number of command address to write
        const uint8_t cmd_send_timeout = 50;    // Timeout for command send
        const uint8_t cmd_send_retries = 3;     // Number of retries for command send
        uint8_t cmd_addr = 0;

        // Mapping color to command address (light location)
        if (color.r == 255 && color.g == 0 && color.b == 0)         cmd_addr = LGSAddress::LED12;   // Red to LED1 and LED2
        else if (color.r == 0 && color.g == 255 && color.b == 0)    cmd_addr = LGSAddress::LED34;   // Green to LED3 and LED4
        else if (color.r == 0 && color.g == 0 && color.b == 255)    cmd_addr = LGSAddress::LED56;   // Blue to LED5 and LED6
        else if (color.r == 255 && color.g == 145 && color.b == 0)  cmd_addr = LGSAddress::LED78;   // Yellow to LED7 and LED8
        else 
        {
            LOG_ERROR_MSG(CAT_LGS, "Invalid color for STANDARD module");
            return false;
        }

        // Send color command
        lgs.writeData(cmd_addr, 0, r);
        lgs.writeData(cmd_addr, 1, g);
        lgs.writeData(cmd_addr, 2, b);
        return lgs.write(module_id, cmd_addr_num, cmd_addr, cmd_send_timeout, cmd_send_retries);
    }
    return true;
}
