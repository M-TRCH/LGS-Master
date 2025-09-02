
#ifndef ETHERNET_UTILS_H
#define ETHERNET_UTILS_H

#include <Ethernet.h>
#include "system.h"
#include "config.h"

// tcp server settings
#define TCP_SERVER_PORT     2000
#define CLIENT_TIMEOUT_MS   5000  // timeout in ms

// Struct for storing client information
struct TcpClientInfo 
{
    EthernetClient client;
    String info;
    uint32_t last_active_time;
    bool connected;
};

extern EthernetServer tcp_server;
extern TcpClientInfo tcp_client;

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

#endif
