
#include "tcp_lgs_bridge.h"

void tcp_indicator_update(TcpIndicatorState_t state)
{
    static TcpIndicatorState_t last_state = TCP_INDICATOR_NONE;
    if (state != last_state)
    {
        last_state = state;
        switch (state)
        {
            case TCP_INDICATOR_NONE:
                set_info(cl_clear, device_info); // Turn off LED
                break;
            case TCP_INDICATOR_IDLE:
                set_info(cl_white, device_info);  // Indicate idle with white LED
                break;
            case TCP_INDICATOR_WAITING:
                set_info(cl_yellow, device_info);  // Indicate waiting with yellow LED
                break;
            case TCP_INDICATOR_CONNECTED:
                set_info(cl_green, device_info); // Indicate connected with green LED
                break;
            case TCP_INDICATOR_ERROR:
                set_info(cl_red, device_info);   // Indicate error with red LED
                break;
        }
    }
}

void tcp_command_execute(const LGSTCPCommand_t cmd, const int color, const int quantity)
{
   
}