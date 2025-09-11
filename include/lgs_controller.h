#ifndef LGS_CONTROLLER_H
#define LGS_CONTROLLER_H

#include <Arduino.h>
#include "LGSBus.h"
#include "system.h"

// LGS command address (Refer to Documentation)
struct LGSAddress 
{
    static constexpr int LGS_GREET  = 0;
    static constexpr int LGS_LED1   = 1;
    static constexpr int LGS_LED2   = 2;
    static constexpr int LGS_LED3   = 3;
    static constexpr int LGS_LED4   = 4;
    static constexpr int LGS_LED5   = 5;
    static constexpr int LGS_LED6   = 6;
    static constexpr int LGS_LED7   = 7;
    static constexpr int LGS_LED8   = 8;
    static constexpr int LGS_LED12  = 9;
    static constexpr int LGS_LED34  = 10;
    static constexpr int LGS_LED56  = 11;
    static constexpr int LGS_LED78  = 12;
};

// Color structure to hold RGB values
struct ModuleColor 
{
    int r = 0;   // Red value (0-255)
    int g = 0;   // Green value (0-255)
    int b = 0;   // Blue value (0-255)

    ModuleColor() = default;
    ModuleColor(int red, int green, int blue) : r(red), g(green), b(blue) {}
};

// Module address structure
struct ModuleAddress 
{
    int id = -1;        // Module ID (if used), otherwise -1
    int row = -1;       // Row number (1-8 for standard, 0-9 for narcotic), otherwise -1
    int col = -1;       // Column number, otherwise -1

    // Construct from module ID
    ModuleAddress(int module_id) : id(module_id), row(-1), col(-1) {}

    // Construct from row and column
    ModuleAddress(int r, int c) : id(-1), row(r), col(c) {}

    // Get module ID if available, or calculate from row/col
    int get_id() const 
    {
        if (id != -1) return id;
        if (row != -1 && col != -1) return row * 10 + col;
        return -1; // Invalid address
    }
};

// Module status enumeration
typedef enum    
{
    MODULE_ERROR = 0,
    MODULE_IDLE,
    MODULE_BUSY
} ModuleStatus_t;

extern LGSbus lgs;
extern ModuleColor cl_clear;
extern ModuleColor cl_red;
extern ModuleColor cl_green;
extern ModuleColor cl_blue;
extern ModuleColor cl_yellow;
extern ModuleColor cl_white;

/*
 * @brief Initialize the LGS communication bus
 */
void lgs_init();

/**
 * @brief Set device information (color indicator and firmware version)
 * @param color ModuleColor for LED indicator
 * @param info DeviceInfo_t containing IP address and firmware version
 * @return true if successful, false otherwise
 */
bool set_info(const ModuleColor& color, const DeviceInfo_t& info);

/**
 * @brief Set color for a module using ModuleInfo
 * @param info ModuleInfo containing type and address
 * @param color ModuleColor containing RGB values
 * @param state true to turn on, false to turn off
 */
bool set_color(const ModuleType& type, const ModuleAddress& addr, const ModuleColor& color, float brightness=1.0, bool state=false, int quantity=0);

/**
 * @brief Request status from a module
 * @param type ModuleType (STANDARD or NARCOTIC)
 * @param addr ModuleAddress containing module ID or row/column
 * @param color ModuleColor indicating which color to check (for STANDARD modules)
 * @param status Output parameter to hold the module status (IDLE, BUSY, ERROR)
 * @return true if communication was successful, false otherwise
 */
bool request_status(const ModuleType& type, const ModuleAddress& addr, const ModuleColor& color, ModuleStatus_t& status);

/*  
 * @brief Perform a soft reset of the system with LED indication
 */
void soft_reset(bool indicate=true);

/*  
 * @brief Handle white button press event
 */
void white_button_event();

/*  
 * @brief Handle red button press event
 */
void red_button_event();

/*  
 * @brief Handle green button press event
 */
void green_button_event();

/*  
 * @brief Handle blue button press event
 */
void blue_button_event();

/*  
 * @brief Handle yellow button press event
 */
void yellow_button_event();

#endif  /* LGS_CONTROLLER_H */