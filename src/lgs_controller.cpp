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
