#ifndef ETHERNET_UTILS_H
#define ETHERNET_UTILS_H

#include <Ethernet.h>
#include <PubSubClient.h>
#include "system.h"

// Struct for storing client information
struct TcpClientInfo 
{
    EthernetClient client;
    String info;
    uint32_t last_active_time;
    bool connected;
};

// Struct for storing TCP packet data
struct TcpPacket
{
    int cabinet;
    int row;
    int column;
    int quantity;
    int color;
    int command;
    int ret_status;
    int transition;
    int device;
    int sum;
};

// Packet status codes
struct PacketStatus 
{
    static constexpr const int FIRST_SUCCEED    = 1;
    static constexpr const int SECOND_SUCCEED   = 2;
    static constexpr const int NO_ACTION        = 3;
    static constexpr const int IDLE             = 4;
    static constexpr const int BUSY             = 5;
    static constexpr const int FAIL             = 9;
};

// MQTT client info
struct MqttClientInfo 
{
    PubSubClient* client;
    bool connected;
    String last_error;
    uint32_t last_reconnect_attempt;  // Track last reconnect attempt time
};

// MQTT message types
struct MqttMessageType 
{
    static constexpr const char* INFO    = "info";
    static constexpr const char* WARNING = "warning";
    static constexpr const char* ERROR   = "error";
    static constexpr const char* DEBUG   = "debug";
    static constexpr const char* VERBOSE = "verbose";
};

// Ethernet connection status
extern bool ethernet_not_linked;

// tcp server and client instances 
extern uint16_t transition_numbers[ProjectConfig::Network::MAX_DEVICE_COUNT]; // For tracking transition number per device
extern EthernetServer tcp_server;
extern TcpClientInfo tcp_client;
extern TcpPacket tcp_packet;

// MQTT client instances
extern EthernetClient eth_client;
extern PubSubClient mqtt_client;
extern MqttClientInfo mqtt_info;
extern String mqtt_client_id;   // MQTT client ID ("lgs" + IP address without dots e.g. "lgs192168099")
extern String mqtt_topic;       // MQTT topic to publish messages to ("lgs/" + IP address without dots + "/opta" e.g. "lgs/192168099/opta")
extern String mqtt_device_id;   // Device ID (IP address with dots e.g. "192.168.0.99")

/**
 * @brief Initialize Ethernet connection
 */
void ethernet_init();

/**
 * @brief Initialize TCP server
 */
void tcp_server_init();

/**
 * @brief Update TCP server and manage client connection
 * @return true if client connected, false otherwise
 */
bool tcp_server_update();

/**
 * @brief Receive and parse a TCP packet from client.
 * @param packet Reference to TcpPacket struct to fill.
 * @return 1 if valid and new packet, 0 otherwise.
 *
 * Example packet: B03R01C02Q0098L01M03T00N01D4000S09
 * Header:            B
 * Cabinet:           03
 * Row:               01
 * Column:            02
 * Quantity:          0098
 * Color:             01
 * Command:           03 
 * Status Return:     00
 * Transition Number: 01
 * Device:            4520
 * Sum:               28
 *
 * Example packet 2: B01R08C01Q0002L01M01T00N01D0000S15 (row 8, column 1, red on)
 * Example packet 3: B01R08C01Q0002L01M00T00N02D0000S15 (row 8, column 1, red off)
 * Example packet 4: B01R08C01Q0002L01M02T00N03D0000S18 (row 8, column 1, red idle?)
 * Example packet 5: B01R00C00Q0000L01M09T00N05D0000S16 (hardware reset)
 */
int receive_tcp_packet(TcpPacket &packet);

/**
 * @brief Send a TCP packet to client.
 * @param packet Reference to TcpPacket struct to send.
 * @return 1 if sent successfully, 0 otherwise.
 */
int return_tcp_packet(TcpPacket& packet);

/**
 * @brief Initialize MQTT client and connect to broker.
 * @return true if connected, false otherwise.
 */
bool mqtt_init();

/**
 * @brief Update MQTT client connection.
 */
void mqtt_update();

/**
 * @brief Publish a JSON-formatted message to MQTT broker.
 * @param level Message level (use MqttMessageType constants)
 * @param message Message content
 * @param topic MQTT topic to publish to (default is mqtt_topic)
 * @return true if published successfully, false otherwise.
 */
bool mqtt_publish_json(const char* level, const String& message, const char* topic=mqtt_topic.c_str());

/**
 * @brief Publish TCP packet details to MQTT broker in JSON format.
 * @param packet Reference to TcpPacket struct to publish.
 * @param type Message type (use MqttMessageType constants)
 * @param prefix Optional prefix to add to the message content
 * @return true if published successfully, false otherwise.
 */
bool publish_tcp_packet(const TcpPacket& packet, const char* type, const String& prefix);

#endif
