
#include "ethernet_utils.h"

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
