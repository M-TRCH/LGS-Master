
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
        if (packet.row < ProjectConfig::Lgs::STANDARD_MIN_ROW || packet.row > ProjectConfig::Lgs::STANDARD_MAX_ROW ||
            packet.column < ProjectConfig::Lgs::STANDARD_MIN_COLUMN || packet.column > ProjectConfig::Lgs::STANDARD_MAX_COLUMN)
        {
            LOG_ERROR_F(CAT_LGS, "Invalid row/column for standard module: row=%d, col=%d", packet.row, packet.column);
            return false; // Invalid row/column for standard module
        }

        // Validate color
        if (packet.color < ProjectConfig::Lgs::STANDARD_MIN_COLOR || packet.color > ProjectConfig::Lgs::STANDARD_MAX_COLOR)
        {
            LOG_ERROR_F(CAT_LGS, "Invalid color for standard module: color=%d", packet.color);
            return false; // Invalid color
        }

        // Convert to LGS addressing (row 1-8 to 8-1)
        ModuleAddress addr(ProjectConfig::Protocol::STANDARD_ROW_OFFSET - packet.row, packet.column);
        
        // Determine color
        ModuleColor color;
        if (packet.color == ProjectConfig::Protocol::COLOR_RED)         color = cl_red;
        else if (packet.color == ProjectConfig::Protocol::COLOR_GREEN)  color = cl_green;
        else if (packet.color == ProjectConfig::Protocol::COLOR_BLUE)   color = cl_blue;
        else if (packet.color == ProjectConfig::Protocol::COLOR_YELLOW) color = cl_yellow;
        
        // Execute command
        switch (packet.command)
        {
            case CMD_ON:
            {
                if (set_color(ModuleType::STANDARD, addr, color, ProjectConfig::Lgs::STANDARD_BRIGHTNESS, true))
                {
                    tcp_packet.ret_status = PacketStatus::SECOND_SUCCEED;    
                    LOG_INFO_F(CAT_LGS, "'ON' executed at [%d, %d] with color %d", packet.row, packet.column, packet.color);
                }
                else
                {
                    tcp_packet.ret_status = PacketStatus::FAIL;  
                    LOG_ERROR_F(CAT_LGS, "'ON' failed at [%d, %d] with color %d", packet.row, packet.column, packet.color);
                }
                break;
            }
            case CMD_OFF:
            {
                if (set_color(ModuleType::STANDARD, addr, color, ProjectConfig::Lgs::STANDARD_BRIGHTNESS, false))
                {
                    tcp_packet.ret_status = PacketStatus::SECOND_SUCCEED;    
                    LOG_INFO_F(CAT_LGS, "'OFF' executed at [%d, %d] with color %d", packet.row, packet.column, packet.color);
                }
                else
                {
                    tcp_packet.ret_status = PacketStatus::FAIL;  
                    LOG_ERROR_F(CAT_LGS, "'OFF' failed at [%d, %d] with color %d", packet.row, packet.column, packet.color);
                }
                break;
            }
            case CMD_REQUEST:
            {
                ModuleStatus_t status;
                if (request_status(ModuleType::STANDARD, addr, color, status))
                {
                    if (status == ModuleStatus_t::MODULE_IDLE)
                        tcp_packet.ret_status = PacketStatus::IDLE; // Idle
                    else if (status == ModuleStatus_t::MODULE_BUSY)
                        tcp_packet.ret_status = PacketStatus::BUSY; // Busy
                    else
                        tcp_packet.ret_status = PacketStatus::FAIL; // Error

                    LOG_INFO_F(CAT_LGS, "'REQUEST' executed at [%d, %d] with color %d: status=%d",
                        packet.row, packet.column, packet.color, tcp_packet.ret_status);
                }
                else
                {
                    tcp_packet.ret_status = PacketStatus::FAIL;
                    LOG_ERROR_F(CAT_LGS, "'REQUEST' failed at [%d, %d] with color %d", packet.row, packet.column, packet.color);
                }
                break;
            }
            case CMD_REBOOT:
            {
                LOG_INFO_MSG(CAT_LGS, "'REBOOT' command executed, system resetting...");
                soft_reset(true); // Perform soft reset with LED indication
                break;
            }
            default:
            {
                return false; // Unknown command
            }
        }
    }
    else if (device_type == ModuleType::NARCOTIC)
    {
        // Validate row and column
        if (packet.row < ProjectConfig::Lgs::NARCOTIC_MIN_ROW || packet.row > ProjectConfig::Lgs::NARCOTIC_MAX_ROW ||
            packet.column < ProjectConfig::Lgs::NARCOTIC_MIN_COLUMN || packet.column > ProjectConfig::Lgs::NARCOTIC_MAX_COLUMN)
        {
            LOG_ERROR_F(CAT_LGS, "Invalid row/column for narcotic module: row=%d, col=%d", packet.row, packet.column);
            return false; // Invalid row/column for narcotic module
        }

        // Validate color
        if (packet.color != ProjectConfig::Lgs::NARCOTIC_REQUIRED_COLOR)
        {
            LOG_ERROR_F(CAT_LGS, "Invalid color for narcotic module: color=%d", packet.color);
            return false; // Invalid color
        }

        // convert to LGS addressing (row 1-10 to 9-0)
        ModuleAddress addr(ProjectConfig::Protocol::NARCOTIC_ROW_OFFSET - packet.row, packet.column);

        // Determine color (only red supported)
        ModuleColor color = cl_red;

        // Execute command
        switch (packet.command)
        {
            case CMD_ON:
            {
                if (set_color(ModuleType::NARCOTIC, addr, color, ProjectConfig::Lgs::NARCOTIC_ON_BRIGHTNESS, true, packet.quantity))
                {
                    tcp_packet.ret_status = PacketStatus::SECOND_SUCCEED;    
                    LOG_INFO_F(CAT_LGS, "'ON' executed at [%d, %d] with quantity %d", packet.row, packet.column, packet.quantity);
                }
                else
                {
                    tcp_packet.ret_status = PacketStatus::FAIL;  
                    LOG_ERROR_F(CAT_LGS, "'ON' failed at [%d, %d] with quantity %d", packet.row, packet.column, packet.quantity);
                }
                break;
            }
            case CMD_OFF:
            {
                ModuleStatus_t status = ModuleStatus_t::MODULE_UNKNOWN;
                if (set_color(ModuleType::NARCOTIC, addr, color, ProjectConfig::Lgs::OFF_BRIGHTNESS, false, 0, status))
                {
                    if (status == ModuleStatus_t::MODULE_IDLE)
                        tcp_packet.ret_status = PacketStatus::SECOND_SUCCEED;   // Successfully turned off
                    else 
                        tcp_packet.ret_status = PacketStatus::FAIL;             // Cannot turn off, module busy
                    LOG_INFO_F(CAT_LGS, "'OFF' executed at [%d, %d], Status=%d", packet.row, packet.column, status);
                }
                else
                {
                    if (status == ModuleStatus_t::MODULE_BUSY)
                        tcp_packet.ret_status = PacketStatus::NO_ACTION;    // Cannot turn off, module busy
                    else
                        tcp_packet.ret_status = PacketStatus::FAIL;         // Error
                    LOG_ERROR_F(CAT_LGS, "'OFF' failed at [%d, %d], Status=%d", packet.row, packet.column, status);
                }
                break;
            }
            case CMD_REQUEST:
            {
                ModuleStatus_t status = ModuleStatus_t::MODULE_UNKNOWN;
                if (request_status(ModuleType::NARCOTIC, addr, color, status))
                {
                    if (status == ModuleStatus_t::MODULE_IDLE)
                        tcp_packet.ret_status = PacketStatus::IDLE;     // Idle
                    else if (status == ModuleStatus_t::MODULE_BUSY)
                        tcp_packet.ret_status = PacketStatus::BUSY;     // Busy
                    else
                        tcp_packet.ret_status = PacketStatus::FAIL;     // Error

                    LOG_INFO_F(CAT_LGS, "'REQUEST' executed at [%d, %d]: status=%d",
                        packet.row, packet.column, tcp_packet.ret_status);
                }
                else
                {
                    tcp_packet.ret_status = PacketStatus::FAIL;
                    LOG_ERROR_F(CAT_LGS, "'REQUEST' failed at [%d, %d]", packet.row, packet.column);
                }
                break;
            }
            case CMD_REBOOT:
            {
                LOG_INFO_MSG(CAT_LGS, "'REBOOT' command executed, system resetting...");
                soft_reset(true); // Perform soft reset with LED indication
                break;
            }
        }
    }
    return true;   
}