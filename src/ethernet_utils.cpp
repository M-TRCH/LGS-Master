#include "ethernet_utils.h"

EthernetServer tcp_server(TCP_SERVER_PORT);
TcpClientInfo tcp_client = {EthernetClient(), "", 0, false};
TcpPacket tcp_packet = {0,0,0,0,0,0,0,0,0,0};
uint16_t transition_numbers[MAX_DEVICE] = {0}; // Track transition number for each device

void ethernet_init()
{
    // Initialize Ethernet with static IP only
    PRINT(DEBUG_BASIC, F("Ethernet initializing with static IP\n"));

    // Get IP configuration from device_info struct
    int ip1 = device_info.ip_address.ip1;
    int ip2 = device_info.ip_address.ip2;
    int ip3 = device_info.ip_address.ip3;
    int ip4 = device_info.ip_address.ip4;
    IPAddress ip(ip1, ip2, ip3, ip4);
    IPAddress gateway(ip1, ip2, ip3, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(8, 8, 8, 8);
    
    // Initialize with static IP configuration
    Ethernet.begin(ip, dns, gateway, subnet);

    // Print IP address
    PRINT(DEBUG_VERBOSE, F("IP Address: "));
    PRINT(DEBUG_VERBOSE, Ethernet.localIP());
    
    // Print MAC address
    byte mac[6];
    Ethernet.MACAddress(mac);
    PRINT(DEBUG_VERBOSE, F("\nMAC Address: "));
    for (int i = 0; i < 6; i++) 
    {
        if (mac[i] < 16) PRINT(DEBUG_VERBOSE, F("0"));
        PRINT(DEBUG_VERBOSE, String(mac[i], HEX));
        if (i < 5) PRINT(DEBUG_VERBOSE, F(":"));
    }
    PRINT(DEBUG_VERBOSE, F("\n"));

    // Check link status
    if (Ethernet.linkStatus() == LinkOFF) 
    {
        PRINT(DEBUG_VERBOSE, F("Ethernet cable is not connected.\n"));
    } 
    else 
    {
        PRINT(DEBUG_VERBOSE, F("Ethernet cable connected successfully\n"));
    }
    PRINT(DEBUG_BASIC, F("Ethernet initialized\n"));
}

void tcp_server_init()
{
    // Start the TCP server
    tcp_server.begin();
    PRINT(DEBUG_BASIC, F("TCP server initialized on port "));
    PRINT(DEBUG_BASIC, TCP_SERVER_PORT);
    PRINT(DEBUG_BASIC, F("\n"));
}

void tcp_server_update()
{
    EthernetClient new_client = tcp_server.accept();

    // Accept only one client at a time
    if (!tcp_client.client && new_client) 
    {
        tcp_client.client = new_client;
        tcp_client.connected = true;
        tcp_client.info = "Client connected: " + new_client.remoteIP().toString() + ":" + String(new_client.remotePort());
        PRINT(DEBUG_BASIC, tcp_client.info + F("\n"));
        tcp_client.last_active_time = millis();
    }

    if (tcp_client.client) 
    {
        // If client is not connected, stop and reset
        if (!tcp_client.client.connected()) 
        {
            tcp_client.client.stop();
            tcp_client.connected = false;
            tcp_client.info = "Client disconnected (lost connection)";
            PRINT(DEBUG_BASIC, tcp_client.info + F("\n"));
        }
        // If client is connected but inactive for too long, disconnect
        else if (millis() - tcp_client.last_active_time > CLIENT_TIMEOUT_MS) 
        {
            tcp_client.client.stop();
            tcp_client.connected = false;
            tcp_client.info = "Client disconnected (timeout)";
            PRINT(DEBUG_BASIC, tcp_client.info + F("\n"));
        }
        // If client sent any data, reset timeout
        else if (tcp_client.client.available()) 
        {
            tcp_client.last_active_time = millis();
        }
    }
}

int receive_tcp_packet(TcpPacket &packet)
{
    // Check if the client is connected and has data to read.
    if (!tcp_client.client|| !tcp_client.client.connected() || !tcp_client.client.available())
        return 0;
        
    // Wait for header 'B'
    if (!tcp_client.client.find('B'))
        return 0;

    // Parse and validate all fields
    int fields[10];
    for (int i = 0; i < 10; ++i) 
    {
        if (!tcp_client.client.available()) return 0; // Ensure data is available
        fields[i] = tcp_client.client.parseInt();
    }

    // Assign to struct
    packet.cabinet      = fields[0];
    packet.row          = fields[1];
    packet.column       = fields[2];
    packet.quantity     = fields[3];
    packet.color        = fields[4];
    packet.command      = fields[5];
    packet.ret_status   = fields[6];
    packet.transition   = fields[7];
    packet.device       = fields[8];
    packet.sum          = fields[9];

    // Check device index range to prevent overflow
    if (packet.device < 0 || packet.device >= MAX_DEVICE) 
    {
        PRINT(DEBUG_BASIC, F("Error: Device index out of range\n"));
        return 0;
    }

    // Validate sum and transition number (allow reply only if transition number changed)
    int calc_sum = packet.cabinet + packet.row + packet.column + packet.quantity + packet.color +
                   packet.command + packet.ret_status + packet.transition + packet.device;
    calc_sum = calc_sum % 100;  // Sum is last two digits
    if (packet.sum != calc_sum) 
    {
        PRINT(DEBUG_BASIC, F("Error: Packet sum mismatch\n"));
        return 0;
    }

    if (packet.transition == transition_numbers[packet.device]) 
    {
        PRINT(DEBUG_BASIC, F("Error: Duplicate packet (same transition number)\n"));
        return 0;
    }

    // Update transition number for this device
    transition_numbers[packet.device] = packet.transition;

    // Debug print
    PRINT(DEBUG_VERBOSE, "Packet received: CAB=" + String(packet.cabinet) + ", ROW=" + String(packet.row) +
        ", COL=" + String(packet.column) + ", QTY=" + String(packet.quantity) + ", CLR=" + String(packet.color) +
        ", CMD=" + String(packet.command) + ", RET=" + String(packet.ret_status) + ", TRS=" + String(packet.transition) +
        ", DEV=" + String(packet.device) + ", SUM=" + String(packet.sum) + ", SUM_CAL=" + String(calc_sum) + "\n");

    return 1;
}
