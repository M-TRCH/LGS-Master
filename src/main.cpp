
#include "system.h"
#include "tcp_lgs_bridge.h"

void setup() 
{
#ifdef CONFIG_H
    // Initialize centralized project configuration.
    config_init();
#endif

#ifdef LOGGER_H
    // Initialize logging system first
    logger_init(LOG_VERBOSE);
    // logger_set_category(CAT_MQTT, false); // Disable MQTT category by default
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
    if (!local_mode_active)
    {   
        // Set TCP indicator to idle initially
        tcp_indicator_update(TCP_INDICATOR_IDLE);

        // Initialize Ethernet and TCP server
        ethernet_init();

        if (ethernet_not_linked)
        {   
            // Indicate error if Ethernet cable is not connected
            tcp_indicator_update(TCP_INDICATOR_ERROR); 
        }
        else
        {
            // Initialize TCP server
            tcp_server_init();

            // Initialize MQTT connection
            if (ProjectConfig::Network::MQTT_ENABLED)
            {
                mqtt_init();
            }

            // Indicate waiting for client connection
            tcp_indicator_update(TCP_INDICATOR_WAITING); 
        }
    }
#endif

    // Start watchdog regardless of local or network mode.
    mbed::Watchdog::get_instance().start(ProjectConfig::System::WATCHDOG_TIMEOUT_MS);
}   

void loop() 
{
#ifdef SYSTEM_H
    // Kick watchdog periodically
    if (millis() - kick_watchdog_timer >= ProjectConfig::System::WATCHDOG_FEED_INTERVAL_MS)
    {
        kick_watchdog_timer = millis();
        mbed::Watchdog::get_instance().kick();
    }

    // In local mode, colored buttons run direct panel/module actions.
    if (local_mode_active)
    {
        red_button_event();
        green_button_event();
        blue_button_event();
        yellow_button_event();
    }

    // Always check for white button event to allow reset.
    white_button_event();
#endif

#ifdef ETHERNET_UTILS_H
    if (!local_mode_active && !ethernet_not_linked)
    {
        // Update TCP server and indicator state
        tcp_server_update() ? tcp_indicator_update(TCP_INDICATOR_CONNECTED) : tcp_indicator_update(TCP_INDICATOR_WAITING);

        // Update MQTT connection
        if (ProjectConfig::Network::MQTT_ENABLED)
        {
            mqtt_update();
        }

        // Check for incoming TCP packets
        if (receive_tcp_packet(tcp_packet))
        {
            // Log received packet details
            publish_tcp_packet(tcp_packet, MqttMessageType::INFO, F("Received packet - "));
        
            // Process and respond to the packet
            if (return_tcp_packet(tcp_packet))
            {
                // Log sent packet details
                publish_tcp_packet(tcp_packet, MqttMessageType::INFO, F("First response packet - "));
            }

            // Execute command from the packet
            if (tcp_command_execute(tcp_packet))
            {
                if (return_tcp_packet(tcp_packet))
                {
                    // Log sent packet details
                    publish_tcp_packet(tcp_packet, MqttMessageType::INFO, F("Second response packet - "));
                }
            }
            else
            {
                LOG_ERROR_F(CAT_LGS, "Failed to execute TCP command: cmd=%d, row=%d, col=%d, color=%d", 
                    tcp_packet.command, tcp_packet.row, tcp_packet.column, tcp_packet.color);
            }
        }
    }
#endif
}

