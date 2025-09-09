
#ifndef TCP_LGS_BRIDGE_H
#define TCP_LGS_BRIDGE_H

#include <Arduino.h>
#include "lgs_controller.h"
#include "ethernet_utils.h"

// TCP indicator states
typedef enum 
{
    TCP_INDICATOR_NONE = 0,
    TCP_INDICATOR_IDLE,
    TCP_INDICATOR_WAITING,
    TCP_INDICATOR_CONNECTED,
    TCP_INDICATOR_ERROR 
} TcpIndicatorState_t;

/*  
 * @brief Update TCP indicator LED based on connection state
 */
void tcp_indicator_update(TcpIndicatorState_t state);

/*  
 * @brief Execute TCP command to control modules
 * @param cmd LGSTCPCommand_t command (CMD_ON, CMD_OFF, CMD_RETURN)
 * @param color Color code (1=Red, 2=Green, 3=Blue, 4=Yellow)
 * @param quantity Quantity for CMD_ON/CMD_OFF, ignored for CMD_RETURN
 */
void tcp_command_execute(const LGSTCPCommand_t cmd, const int color, const int quantity);

#endif /* TCP_LGS_BRIDGE_H */  