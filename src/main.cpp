
#include "system.h"
#include "config.h"
#include "ethernet_utils.h"
#include "logger.h"
#include "lgs_controller.h"

/*  
 * @brief Handle red button press event
 */
void red_button_event()
{
    if (debounce_sw(R_SW_PIN))
    {
        set_info(cl_red, device_info);  // Indicate config mode with red LED

        if (device_type == ModuleType::STANDARD)
        {
            for (uint8_t row = 1; row <= 8; row++)
            {
                mbed::Watchdog::get_instance().kick();

                for (uint8_t col = 1; col <= 8; col++)
                {
                    bool red_success = set_color(ModuleType::STANDARD, ModuleAddress(row, col), cl_red, 1.0, true);
                    bool green_success = set_color(ModuleType::STANDARD, ModuleAddress(row, col), cl_green, 1.0, true);
                    bool blue_success = set_color(ModuleType::STANDARD, ModuleAddress(row, col), cl_blue, 1.0, true);
                    bool yellow_success = set_color(ModuleType::STANDARD, ModuleAddress(row, col), cl_yellow, 1.0, true);
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
           
        }

        set_info(cl_clear, device_info); // Clear LED after config
    }    
}

/*  
 * @brief Handle green button press event
 */
void green_button_event()
{
    if (debounce_sw(G_SW_PIN))
    {   
        set_info(cl_green, device_info);  // Indicate config mode with green LED

        if (device_type == ModuleType::STANDARD)
        {
            for (uint8_t row = 1; row <= 8; row++)
            {
                mbed::Watchdog::get_instance().kick();
                
                for (uint8_t col = 1; col <= 8; col++)
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

        }

        set_info(cl_clear, device_info); // Clear LED after config
    }
}

/*
// (3) Functions
void RESET_Event(unsigned long preResetTime = 3000, unsigned long postResetTime = 1000)
{
    setInfo(5, 0, VERSION_DD, VERSION_MM, VERSION_YY); // white
    delay(preResetTime);
    PWR_RELAY2(LOW);
    PWR_RELAY4(LOW);      
    delay(postResetTime);
    NVIC_SystemReset();
}

void W_SW_Event()
{
    if (W_SW) 
    {
        delay(50);
        if (W_SW)
        {
            RESET_Event();
        }
    }  
}
 
void R_SW_Event()
{
    if (R_SW) 
    {
        delay(50);
        if (R_SW)
        {
            setInfo(1, 1, VERSION_DD, VERSION_MM, VERSION_YY);

            #ifdef LGS_STANDARD
                for (int cl=1; cl<=4; cl++)
                {
                    for (int r=1; r<=8; r++)
                    {
                        for (int c=1; c<=8; c++)
                        {
                            std_setColor(r, c, cl);
                        }
                    }
                }
            #endif

            #ifdef LGS_NARCOTIC
                for (int c=1; c<=8; c++)
                {
                    for (int r=0; r<=9; r++)  
                    {
                        LOG_DEBUG_F(CAT_LGS, "Setting active [%d, %d]: %d", r, c, nct_setActive(r, c, 1, 50, 0));
                        delay(1500);
                    }
                    for (int r=0; r<=9; r++)  
                    {
                        LOG_DEBUG_F(CAT_LGS, "Deactivating [%d, %d]: %d", r, c, nct_setActive(r, c));
                        delay(1500);
                    }
                }
            #endif

            setInfo(0, 0, VERSION_DD, VERSION_MM, VERSION_YY);
        }
    }
}

void G_SW_Event()
{
    if (G_SW) 
    {
        delay(50);
        if (G_SW)
        {
            setInfo(2, 2, VERSION_DD, VERSION_MM, VERSION_YY);

            #ifdef LGS_STANDARD
                for (int cl=1; cl<=4; cl++)
                {
                    for (int r=1; r<=8; r++)
                    {
                        for (int c=1; c<=8; c++)
                        {
                            std_setColor(r, c, cl, false);
                        }
                    }
                }
            #endif

            #ifdef LGS_NARCOTIC
                delay(3000);
            #endif

            setInfo(0, 0, VERSION_DD, VERSION_MM, VERSION_YY);
        }
    }
}

void B_SW_Event()
{
    if (B_SW) 
    {
        delay(50);
        if (B_SW)
        {
            setInfo(3, 3, VERSION_DD, VERSION_MM, VERSION_YY);
      
            #ifdef LGS_STANDARD
                delay(3000);
            #endif 

            #ifdef LGS_NARCOTIC
                nct_allUnlock();
            #endif

            setInfo(0, 0, VERSION_DD, VERSION_MM, VERSION_YY);
        }
    }
}

void Y_SW_Event()
{
    if (Y_SW) 
    {
        delay(50);
        if (Y_SW)
        {   
            setInfo(4, 4, VERSION_DD, VERSION_MM, VERSION_YY);
    
            #ifdef LGS_STANDARD
                delay(3000);
            #endif 

            #ifdef LGS_NARCOTIC
                nct_allLock();
            #endif

            setInfo(0, 0, VERSION_DD, VERSION_MM, VERSION_YY);
        }
    }
}

bool run()
{
    // 0) initialize variables
    bool getPacket = false;

    // 1) Get packet from client.
    if (receivePacket())  
    {
        getPacket = true;
        int ret = 0;

        // (2) Take action based on the command.
        #ifdef LGS_STANDARD
            // Invert bin location and convert to physical id.
            // from (1 to 8) ->  new (8 to 1)
            // Serial.println("(" + String(9-ROW) + (", ") + String(COL) + ")");
            switch (CMD)
            {
                case 0:
                    //  Set color off. 
                    ret = std_setColor(9-ROW, COL, CLR, false);
                    if (ret)    returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_SECOND_SUCCEED, TRS[DEV], DEV);   
                    else        returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV);   
                    break;

                case 1: 
                    //  Set color on.
                    ret = std_setColor(9-ROW, COL, CLR);
                    if (ret)    returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_SECOND_SUCCEED, TRS[DEV], DEV);   
                    else        returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV);   
                    break;

                case 2:
                    //  Check if the device is idle or busy.
                    //  If idle, return PACK_IDLE.
                    //  If busy, return PACK_BUSY.
                    //  If error, return PACK_FAIL.  
                    ret = std_requestStatus(9-ROW, COL, CLR);
                    if (ret == 1)       returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_IDLE, TRS[DEV], DEV);  
                    else if (ret == 2)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_BUSY, TRS[DEV], DEV);  
                    else                returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV); 
                    break;

                case 9:
                    //  Hardware reset.
                    RESET_Event();
                    break;
            }
        #endif

        #ifdef LGS_NARCOTIC
        // Convert bin location to physical id.
        // from (1 to 10) ->  new (9 to 0)
        // Serial.println("(" + String(10-ROW) + (", ") + String(COL) + ")");

            switch (CMD)
            {
                case 0: // off
                    ret = nct_setActive(10-ROW, COL);
                    if (ret == 1)       returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_NO_ACTION, TRS[DEV], DEV);   
                    else if (ret == 2)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_SECOND_SUCCEED, TRS[DEV], DEV);   
                    else                returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV);     
                    break;
                
                case 1: // on
                    ret = nct_setActive(10-ROW, COL, CLR, 50, QTY);  // brightness: 50 
                    if (ret)    returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_SECOND_SUCCEED, TRS[DEV], DEV);   
                    else        returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV);
                    break;

                case 2: // request status
                    ret = nct_requestStatus(10-ROW, COL);
                    if (ret == 1)       returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_IDLE, TRS[DEV], DEV);  
                    else if (ret == 2)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_BUSY, TRS[DEV], DEV);  
                    else                returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV); 
                    break;

                case 9: // hardware reset
                    RESET_Event();
                break;
            }
        #endif
    }
    return getPacket;
}

*/

void setup() 
{
    #ifdef LOGGER_H
        // Initialize logging system first
        logger_init(LOG_VERBOSE);
    #endif

    #ifdef SYSTEM_H
        // Initialize system
        system_init();
    #endif

    #ifdef CONFIG_H
        // Initialize configuration with module type, IP address, and firmware version
        config_init(ModuleType::STANDARD, 
            192, 168, 0, 99, 
            8, 9, 2025);
    #endif

    #ifdef LGS_CONTROLLER_H
        // Initialize LGS controller
        lgs_init();
    #endif

    #ifdef ETHERNET_UTILS_H
        // Initialize Ethernet and TCP server
        ethernet_init();

        // Initialize TCP server
        tcp_server_init();

        // Initialize MQTT connection
        mqtt_init();
    #endif

    // Start watchdog if not in dev mode
    if (dev_mode_activated) 
    {

    }
    else 
    {
        mbed::Watchdog::get_instance().start(WATCHDOG_TIMEOUT);
    }
}

void loop() 
{
    #ifdef SYSTEM_H
        // Kick watchdog periodically
        if (millis() - kick_watchdog_timer >= WATCHDOG_FEED_INTERVAL)
        {
            kick_watchdog_timer = millis();
            mbed::Watchdog::get_instance().kick();
        }

        red_button_event();
        green_button_event();
    #endif

    #ifdef ETHERNET_UTILS_H
        // Update TCP server and manage client connection    
        tcp_server_update();
        
        // Update MQTT connection
        mqtt_update();

        // Check for incoming TCP packets
        if (receive_tcp_packet(tcp_packet))
        {
            // Log received packet details
            publish_tcp_packet(tcp_packet, MqttMessageType::INFO, F("Received packet - "));
        
            // Echo back with FIRST_SUCCEED status.
            tcp_packet.ret_status = PacketStatus::FIRST_SUCCEED;    
            
            // Process and respond to the packet
            bool success = return_tcp_packet(tcp_packet);
            if (success)
            {
                // Log sent packet details
                publish_tcp_packet(tcp_packet, MqttMessageType::INFO, F("Sent packet - "));
            }
        }
    #endif

    if (debounce_sw(W_SW_PIN))
    {
        NVIC_SystemReset();
    }
}

