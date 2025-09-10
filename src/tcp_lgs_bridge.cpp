
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

bool tcp_command_execute(const TcpPacket packet)
{
    if (device_type == ModuleType::STANDARD)
    {
        // Validate row and column
        if (packet.row < 1 || packet.row > 8 || packet.column < 1 || packet.column > 8)
        {
            LOG_ERROR_F(CAT_LGS, "Invalid row/column for standard module: row=%d, col=%d", packet.row, packet.column);
            return false; // Invalid row/column for standard module
        }

        // Validate color
        if (packet.color < 1 || packet.color > 4)
        {
            LOG_ERROR_F(CAT_LGS, "Invalid color for standard module: color=%d", packet.color);
            return false; // Invalid color
        }

        // Convert to LGS addressing (row 1-8 to 8-1)
        ModuleAddress addr(9 - packet.row, packet.column);
        
        // Determine color
        ModuleColor color;
        if (packet.color == 1)         color = cl_red;
        else if (packet.color == 2)    color = cl_green;
        else if (packet.color == 3)    color = cl_blue;
        else if (packet.color == 4)    color = cl_yellow;
        
        // Execute command
        switch (packet.command)
        {
            case CMD_ON:
                if (set_color(ModuleType::STANDARD, addr, color, 1.0, true))
                {
                    tcp_packet.ret_status = PacketStatus::SECOND_SUCCEED;    
                    LOG_INFO_F(CAT_LGS, "CMD_ON executed at [%d, %d] with color %d", packet.row, packet.column, packet.color);
                }
                else
                {
                    tcp_packet.ret_status = PacketStatus::FAIL;  
                    LOG_ERROR_F(CAT_LGS, "CMD_ON failed at [%d, %d] with color %d", packet.row, packet.column, packet.color);
                }
                break;

            case CMD_OFF:
                if (set_color(ModuleType::STANDARD, addr, color, 1.0, false))
                {
                    tcp_packet.ret_status = PacketStatus::SECOND_SUCCEED;    
                    LOG_INFO_F(CAT_LGS, "CMD_OFF executed at [%d, %d] with color %d", packet.row, packet.column, packet.color);
                }
                else
                {
                    tcp_packet.ret_status = PacketStatus::FAIL;  
                    LOG_ERROR_F(CAT_LGS, "CMD_OFF failed at [%d, %d] with color %d", packet.row, packet.column, packet.color);
                }
                break;

            case CMD_RETURN:
                ModuleStatus_t status;
                if (request_status(ModuleType::STANDARD, addr, color, status))
                {
                    if (status == ModuleStatus_t::MODULE_IDLE)
                        tcp_packet.ret_status = PacketStatus::IDLE; // Idle
                    else if (status == ModuleStatus_t::MODULE_BUSY)
                        tcp_packet.ret_status = PacketStatus::BUSY; // Busy
                    else
                        tcp_packet.ret_status = PacketStatus::FAIL; // Error
                    
                    LOG_INFO_F(CAT_LGS, "CMD_RETURN executed at [%d, %d] with color %d: status=%d", 
                        packet.row, packet.column, packet.color, tcp_packet.ret_status);
                }
                else
                {
                    tcp_packet.ret_status = PacketStatus::FAIL;  
                    LOG_ERROR_F(CAT_LGS, "CMD_RETURN failed at [%d, %d] with color %d", packet.row, packet.column, packet.color);
                }
                break;

            default:
                return false; // Unknown command
        }
    }
    else if (device_type == ModuleType::NARCOTIC)
    {
        // Handle narcotic module commands
    }
    return true;   
}