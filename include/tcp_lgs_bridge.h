
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

// TCP command types
typedef enum LGSTCPCommand
{
    CMD_OFF     = 0,
    CMD_ON      = 1,
    CMD_REQUEST = 2,
    CMD_REBOOT  = 9
} LGSTCPCommand_t;

/*  
 * @brief Update TCP indicator LED based on connection state
 */
void tcp_indicator_update(TcpIndicatorState_t state);

/*  
 * @brief Execute TCP command to control modules
 * @param packet tcp_packet containing command details
 * @return true if command executed successfully, false otherwise
 */
bool tcp_command_execute(const TcpPacket packet);

#endif /* TCP_LGS_BRIDGE_H */  