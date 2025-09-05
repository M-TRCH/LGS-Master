#include "ethernet_utils.h"

// tcp server and client instances
uint16_t transition_numbers[MAX_DEVICE] = {0}; // Track transition number for each device
EthernetServer tcp_server(TCP_SERVER_PORT);
TcpClientInfo tcp_client = {EthernetClient(), "", 0, false};
TcpPacket tcp_packet = {0,0,0,0,0,0,0,0,0,0};

// MQTT client instances
EthernetClient eth_client;
PubSubClient mqtt_client(eth_client);
MqttClientInfo mqtt_info = {&mqtt_client, false, ""};

void ethernet_init()
{
    // Initialize Ethernet with static IP only
    LOG_INFO_MSG(CAT_NETWORK, "Ethernet initializing with static IP");

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

    // Log IP address
    LOG_VERBOSE_F(CAT_NETWORK, "IP Address: %d.%d.%d.%d", 
                  Ethernet.localIP()[0], Ethernet.localIP()[1], 
                  Ethernet.localIP()[2], Ethernet.localIP()[3]);
    
    // Log MAC address
    byte mac[6];
    Ethernet.MACAddress(mac);
    LOG_VERBOSE_F(CAT_NETWORK, "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Check link status
    if (Ethernet.linkStatus() == LinkOFF) 
    {
        LOG_WARN_MSG(CAT_NETWORK, "Ethernet cable is not connected");
    } 
    else 
    {
        LOG_INFO_MSG(CAT_NETWORK, "Ethernet cable connected successfully");
    }
    LOG_INFO_MSG(CAT_NETWORK, "Ethernet initialized successfully");
}

void tcp_server_init()
{
    // Start the TCP server
    tcp_server.begin();
    LOG_INFO_F(CAT_TCP, "TCP server initialized on port %d", TCP_SERVER_PORT);
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
        LOG_INFO_MSG(CAT_TCP, tcp_client.info);
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
            LOG_INFO_MSG(CAT_TCP, tcp_client.info);
        }
        // If client is connected but inactive for too long, disconnect
        else if (millis() - tcp_client.last_active_time > CLIENT_TIMEOUT_MS) 
        {
            tcp_client.client.stop();
            tcp_client.connected = false;
            tcp_client.info = "Client disconnected (timeout)";
            LOG_WARN_MSG(CAT_TCP, tcp_client.info);
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
        LOG_ERROR_MSG(CAT_TCP, "Device index out of range");
        return 0;
    }

    // Validate sum and transition number (allow reply only if transition number changed)
    int calc_sum = packet.cabinet + packet.row + packet.column + packet.quantity + packet.color +
                   packet.command + packet.ret_status + packet.transition + packet.device;
    calc_sum = calc_sum % 100;  // Sum is last two digits
    if (packet.sum != calc_sum) 
    {
        LOG_ERROR_F(CAT_TCP, "Packet checksum mismatch - received: %d, calculated: %d", packet.sum, calc_sum);
        return 0;
    }

    if (packet.transition == transition_numbers[packet.device]) 
    {
        LOG_WARN_F(CAT_TCP, "Duplicate packet detected (device %d, transition %d)", packet.device, packet.transition);
        return 0;
    }

    // Update transition number for this device
    transition_numbers[packet.device] = packet.transition;

    // Debug print packet details
    LOG_VERBOSE_F(CAT_TCP, "Packet received - Cabinet:%d Row:%d Col:%d Qty:%d Color:%d Cmd:%d Status:%d Trans:%d Device:%d Sum:%d", 
                  packet.cabinet, packet.row, packet.column, packet.quantity, packet.color,
                  packet.command, packet.ret_status, packet.transition, packet.device, packet.sum);

    return 1;
}

int return_tcp_packet(const TcpPacket& packet)
{
    // Check client status
    if (!tcp_client.client || !tcp_client.client.connected()) 
    {
        LOG_ERROR_MSG(CAT_TCP, "No active client for sending packet");
        return 0;
    }

    // Check field size limits
    if (packet.cabinet < 0 || packet.cabinet > 99 ||
        packet.row < 0 || packet.row > 99 ||
        packet.column < 0 || packet.column > 99 ||
        packet.quantity < 0 || packet.quantity > 9999 ||
        packet.color < 0 || packet.color > 99 ||
        packet.command < 0 || packet.command > 99 ||
        packet.ret_status < 0 || packet.ret_status > 99 ||
        packet.transition < 0 || packet.transition > 99 ||
        packet.device < 0 || packet.device > 9998) 
    {
        LOG_ERROR_MSG(CAT_TCP, "Packet field values out of range");
        return 0;
    }

    // Calculate summary (checksum)
    int calc_sum = packet.cabinet + packet.row + packet.column + packet.quantity + packet.color +
                   packet.command + packet.ret_status + packet.transition + packet.device;
    calc_sum = calc_sum % 100;  // Sum is last two digits

    // Send packet
    tcp_client.client.print("B");
    if (packet.cabinet < 10) tcp_client.client.print("0");
    tcp_client.client.print(packet.cabinet);

    tcp_client.client.print("R");
    if (packet.row < 10) tcp_client.client.print("0");
    tcp_client.client.print(packet.row);

    tcp_client.client.print("C");
    if (packet.column < 10) tcp_client.client.print("0");
    tcp_client.client.print(packet.column);

    tcp_client.client.print("Q");
    if (packet.quantity < 1000) tcp_client.client.print("0");
    if (packet.quantity < 100) tcp_client.client.print("0");
    if (packet.quantity < 10) tcp_client.client.print("0");
    tcp_client.client.print(packet.quantity);

    tcp_client.client.print("L");
    if (packet.color < 10) tcp_client.client.print("0");
    tcp_client.client.print(packet.color);

    tcp_client.client.print("M");
    if (packet.command < 10) tcp_client.client.print("0");
    tcp_client.client.print(packet.command);

    tcp_client.client.print("T");
    if (packet.ret_status < 10) tcp_client.client.print("0");
    tcp_client.client.print(packet.ret_status);

    tcp_client.client.print("N");
    if (packet.transition < 10) tcp_client.client.print("0");
    tcp_client.client.print(packet.transition);

    tcp_client.client.print("D");
    if (packet.device < 1000) tcp_client.client.print("0");
    if (packet.device < 100) tcp_client.client.print("0");
    if (packet.device < 10) tcp_client.client.print("0");
    tcp_client.client.print(packet.device);

    tcp_client.client.print("S");
    if (calc_sum < 10) tcp_client.client.print("0");
    tcp_client.client.print(calc_sum);

    tcp_client.client.println();

    // Success
    LOG_VERBOSE_F(CAT_TCP, "Packet sent to client - Cabinet:%d Row:%d Col:%d Qty:%d Color:%d Cmd:%d Status:%d Trans:%d Device:%d Sum:%d", 
                packet.cabinet, packet.row, packet.column, packet.quantity, packet.color,
                packet.command, packet.ret_status, packet.transition, packet.device, calc_sum);

    return 1;
}

bool mqtt_init()
{
    mqtt_client.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
    if (mqtt_client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) 
    {
        mqtt_info.connected = true;
        mqtt_info.last_error = "";
        LOG_INFO_MSG(CAT_MQTT, "Connected to MQTT broker");
        return true;
    } 
    else 
    {
        mqtt_info.connected = false;
        mqtt_info.last_error = "Failed to connect to MQTT broker";
        LOG_ERROR_MSG(CAT_MQTT, mqtt_info.last_error);
        return false;
    }
}

void mqtt_update()
{
    if (!mqtt_client.connected()) 
    {
        LOG_WARN_MSG(CAT_MQTT, "Disconnected, attempting reconnect");
        if (mqtt_client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) 
        {
            mqtt_info.connected = true;
            mqtt_info.last_error = "";
            LOG_INFO_MSG(CAT_MQTT, "Reconnected to broker");
        } 
        else 
        {
            mqtt_info.connected = false;
            mqtt_info.last_error = "Failed to reconnect";
            LOG_ERROR_MSG(CAT_MQTT, mqtt_info.last_error);
        }
    }
    else
    {
        mqtt_info.connected = true;
    }
    mqtt_client.loop(); // Process incoming/outgoing MQTT messages
}

bool mqtt_publish_json(const char* type, const String& message, const char* topic)
{
    if (!mqtt_info.connected) 
    {
        LOG_ERROR_MSG(CAT_MQTT, "Cannot publish - not connected to broker");
        return false;
    }
    
    String json = "{\"type\":\"" + String(type) + "\",\"message\":\"" + message + "\"}";
    bool result = mqtt_client.publish(topic, json.c_str());
    
    if (result) 
    {
        LOG_INFO_F(CAT_MQTT, "Published to topic %s", topic);
        LOG_VERBOSE_MSG(CAT_MQTT, "Published JSON: " + json);
    } 
    else 
    {
        LOG_ERROR_F(CAT_MQTT, "Failed to publish to topic %s", topic);
    }
    return result;
}
