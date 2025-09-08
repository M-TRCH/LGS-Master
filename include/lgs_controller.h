#ifndef LGS_CONTROLLER_H
#define LGS_CONTROLLER_H

#include <Arduino.h>
#include "LGSBus.h"
#include "system.h"
#include "logger.h"

// LGS command address (Refer to Documentation)
struct LGSAddress 
{
    static constexpr int GREET  = 0;
    static constexpr int LED1   = 1;
    static constexpr int LED2   = 2;
    static constexpr int LED3   = 3;
    static constexpr int LED4   = 4;
    static constexpr int LED5   = 5;
    static constexpr int LED6   = 6;
    static constexpr int LED7   = 7;
    static constexpr int LED8   = 8;
    static constexpr int LED12  = 9;
    static constexpr int LED34  = 10;
    static constexpr int LED56  = 11;
    static constexpr int LED78  = 12;
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

// Module type enumeration
enum class ModuleType 
{
    STANDARD,   // Standard module (row 1-8)
    NARCOTIC    // Narcotic module (row 0-9)
};

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
 * @brief Set color for a module using ModuleInfo
 * @param info ModuleInfo containing type and address
 * @param color ModuleColor containing RGB values
 * @param state true to turn on, false to turn off
 */
bool set_color(const ModuleType& type, const ModuleAddress& addr, const ModuleColor& color, float brightness=1.0, bool state=false);

#endif  /* LGS_CONTROLLER_H */