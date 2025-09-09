
#include "system.h"
#include "ethernet_utils.h"
#include "lgs_controller.h"

void setup() 
{
#ifdef CONFIG_H
    // Initialize configuration with module type, IP address, and firmware version
    config_init(ModuleType::STANDARD, 
        192, 168, 0, 99, 
        9, 9, 2025);
#endif

#ifdef LOGGER_H
    // Initialize logging system first
    logger_init(LOG_VERBOSE);
#endif

#ifdef SYSTEM_H
    // Initialize system
    system_init();
#endif

#ifdef LGS_CONTROLLER_H
    // Initialize LGS controller
    lgs_init();
#endif

#ifdef ETHERNET_UTILS_H
    if (!dev_mode_activated)
    {   
        // Indicate network initialization with white LED
        set_info(cl_yellow, device_info);

        // Initialize Ethernet and TCP server
        ethernet_init();

        if (ethernet_not_linked)
        {
            // If Ethernet cable is not connected, indicate with blue LED
            set_info(cl_blue, device_info);
        }
        else
        {
            // Initialize TCP server
            tcp_server_init();

            // Initialize MQTT connection
            mqtt_init();

            // Indicate successful network connection with green LED
            set_info(cl_green, device_info);
        }
    }
#endif

    // Start watchdog if not in dev mode
    mbed::Watchdog::get_instance().start(WATCHDOG_TIMEOUT);
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

    // Check for developer mode activation
    if (dev_mode_activated)
    {
        red_button_event();
        green_button_event();
    }

    // Check for system reset command
    if (debounce_sw(W_SW_PIN))
    {
        NVIC_SystemReset();
    }
#endif

#ifdef ETHERNET_UTILS_H
    if (!dev_mode_activated && !ethernet_not_linked)
    {
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
    }
#endif
}

