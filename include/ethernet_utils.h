#ifndef ETHERNET_UTILS_H
#define ETHERNET_UTILS_H

#include <Ethernet.h>
#include "system.h"
#include "config.h"

// tcp server settings
#define TCP_SERVER_PORT     2000
#define CLIENT_TIMEOUT_MS   30000   // timeout in ms
#define MAX_DEVICE          9999    // maximum number of devices

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

extern EthernetServer tcp_server;
extern TcpClientInfo tcp_client;
extern TcpPacket tcp_packet;
extern uint16_t transition_numbers[MAX_DEVICE]; // For tracking transition number per device

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
 */
void tcp_server_update();

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
 * Example packet 4: B01R08C01Q0002L01M02T00N03D0000S18 (row 8, column 1, red idle)
 * Example packet 5: B01R00C00Q0000L01M09T00N05D0000S16 (hardware reset)
 */
int receive_tcp_packet(TcpPacket &packet);

#endif
