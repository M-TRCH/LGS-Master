#include "ethernet_utils.h"

namespace
{
    IPAddress to_ip_address(const IPAddress_t& ip)
    {
        return IPAddress(ip.ip1, ip.ip2, ip.ip3, ip.ip4);
    }

    void print_padded_int(EthernetClient& client, int value, uint8_t width)
    {
        int divisor = 1;
        for (uint8_t index = 1; index < width; ++index)
        {
            divisor *= 10;
        }

        for (uint8_t index = 1; index < width; ++index)
        {
            if (value < divisor)
            {
                client.print('0');
            }
            divisor /= 10;
        }

        client.print(value);
    }
}

// Ethernet connection status
bool ethernet_not_linked = false; // Track Ethernet link status

// tcp server and client instances
uint16_t transition_numbers[ProjectConfig::Network::MAX_DEVICE_COUNT] = {0}; // Track transition number for each device
EthernetServer tcp_server(ProjectConfig::Network::TCP_SERVER_PORT);
TcpClientInfo tcp_client = {EthernetClient(), "", 0, false};
TcpPacket tcp_packet = {0,0,0,0,0,0,0,0,0,0};

// MQTT client instances
EthernetClient eth_client;
PubSubClient mqtt_client(eth_client);
MqttClientInfo mqtt_info = {&mqtt_client, false, "", 0};
String mqtt_client_id = "";   
String mqtt_topic = "";       
String mqtt_device_id = "";   

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
    IPAddress gateway = to_ip_address(ProjectConfig::Network::GATEWAY);
    IPAddress subnet = to_ip_address(ProjectConfig::Network::SUBNET);
    IPAddress dns = to_ip_address(ProjectConfig::Network::DNS);
    
    // Initialize with static IP configuration
    Ethernet.begin(ip, dns, gateway, subnet);
    
    // Generate MQTT client ID and topic based on IP address
    String ip_dot_str = String(ip1) + "." + String(ip2) + "." + String(ip3) + "." + String(ip4);
    String ip_str = String(ip1) + String(ip2) + String(ip3) + String(ip4);
    mqtt_client_id = String(ProjectConfig::Network::MQTT_CLIENT_PREFIX) + ip_str;            // e.g. "lgs192168099"
    mqtt_topic = String(ProjectConfig::Network::MQTT_TOPIC_PREFIX) + ip_str + ProjectConfig::Network::MQTT_TOPIC_SUFFIX;     // e.g. "lgs/192168099/opta"
    mqtt_device_id = ip_dot_str;                // e.g. "192.168.0.99"

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
        ethernet_not_linked = true;
    } 
    else 
    {
        LOG_INFO_MSG(CAT_NETWORK, "Ethernet cable connected successfully");
        ethernet_not_linked = false;
    }
    LOG_INFO_MSG(CAT_NETWORK, "Ethernet initialized successfully");
}

void tcp_server_init()
{
    // Start the TCP server
    tcp_server.begin();
    LOG_INFO_F(CAT_TCP, "TCP server initialized on port %d", ProjectConfig::Network::TCP_SERVER_PORT);
}

bool tcp_server_update()
{
    bool client_connected = false;
    // Accept new client if available
    EthernetClient new_client = tcp_server.accept();

    // Accept only one client at a time
    if (!tcp_client.client && new_client) 
    {
        tcp_client.client = new_client;
        tcp_client.connected = true;
        tcp_client.info = "Client connected: " + new_client.remoteIP().toString() + ":" + String(new_client.remotePort());
        tcp_client.last_active_time = millis();
        LOG_INFO_MSG(CAT_TCP, tcp_client.info);
        client_connected = true;
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
            client_connected = false;
        }
        // If client is connected but inactive for too long, disconnect
        else if (millis() - tcp_client.last_active_time > ProjectConfig::Network::CLIENT_TIMEOUT_MS) 
        {
            tcp_client.client.stop();
            tcp_client.connected = false;
            tcp_client.info = "Client disconnected (timeout)";
            LOG_WARN_MSG(CAT_TCP, tcp_client.info);
            client_connected = false;
        }
        // If client sent any data, reset timeout
        else if (tcp_client.client.available()) 
        {
            tcp_client.last_active_time = millis();
            client_connected = true;
        }
        // Client still connected but no data
        else
        {
            client_connected = true; 
        }
    }
    return client_connected;
}

int receive_tcp_packet(TcpPacket &packet)
{
    // Check if the client is connected and has data to read.
    if (!tcp_client.client|| !tcp_client.client.connected() || !tcp_client.client.available())
        return 0;
        
    // Wait for header 'B'
    if (!tcp_client.client.find(ProjectConfig::Protocol::PACKET_HEADER))
        return 0;

    // Parse and validate all fields
    int fields[ProjectConfig::Protocol::PACKET_FIELD_COUNT];
    for (int index = 0; index < ProjectConfig::Protocol::PACKET_FIELD_COUNT; ++index) 
    {
        if (!tcp_client.client.available()) return 0; // Ensure data is available
        fields[index] = tcp_client.client.parseInt();
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
    if (packet.device < 0 || packet.device >= static_cast<int>(ProjectConfig::Network::MAX_DEVICE_COUNT)) 
    {
        LOG_ERROR_MSG(CAT_TCP, "Device index out of range");
        return 0;
    }

    // Validate sum and transition number (allow reply only if transition number changed)
    int calc_sum = packet.cabinet + packet.row + packet.column + packet.quantity + packet.color +
                   packet.command + packet.ret_status + packet.transition + packet.device;
    calc_sum = calc_sum % ProjectConfig::Protocol::CHECKSUM_MODULO;
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

    // Update return status as FIRST_SUCCEED.
    tcp_packet.ret_status = PacketStatus::FIRST_SUCCEED;    
            
    // Debug print packet details
    LOG_VERBOSE_F(CAT_TCP, "Packet received - Cabinet:%d Row:%d Col:%d Qty:%d Color:%d Cmd:%d Status:%d Trans:%d Device:%d Sum:%d", 
                  packet.cabinet, packet.row, packet.column, packet.quantity, packet.color,
                  packet.command, packet.ret_status, packet.transition, packet.device, packet.sum);

    return 1;
}

int return_tcp_packet(TcpPacket& packet)
{
    // Check client status
    if (!tcp_client.client || !tcp_client.client.connected()) 
    {
        LOG_ERROR_MSG(CAT_TCP, "No active client for sending packet");
        return 0;
    }

    // Check field size limits
    if (packet.cabinet < 0 || packet.cabinet > ProjectConfig::Protocol::MAX_TWO_DIGIT_VALUE ||
        packet.row < 0 || packet.row > ProjectConfig::Protocol::MAX_TWO_DIGIT_VALUE ||
        packet.column < 0 || packet.column > ProjectConfig::Protocol::MAX_TWO_DIGIT_VALUE ||
        packet.quantity < 0 || packet.quantity > ProjectConfig::Protocol::MAX_FOUR_DIGIT_VALUE ||
        packet.color < 0 || packet.color > ProjectConfig::Protocol::MAX_TWO_DIGIT_VALUE ||
        packet.command < 0 || packet.command > ProjectConfig::Protocol::MAX_TWO_DIGIT_VALUE ||
        packet.ret_status < 0 || packet.ret_status > ProjectConfig::Protocol::MAX_TWO_DIGIT_VALUE ||
        packet.transition < 0 || packet.transition > ProjectConfig::Protocol::MAX_TWO_DIGIT_VALUE ||
        packet.device < 0 || packet.device > ProjectConfig::Protocol::MAX_DEVICE_ID) 
    {
        LOG_ERROR_MSG(CAT_TCP, "Packet field values out of range");
        return 0;
    }

    // Calculate summary (checksum)
    packet.sum = packet.cabinet + packet.row + packet.column + packet.quantity + packet.color +
                   packet.command + packet.ret_status + packet.transition + packet.device;
    packet.sum = packet.sum % ProjectConfig::Protocol::CHECKSUM_MODULO;

    // Send packet
    tcp_client.client.print(ProjectConfig::Protocol::PACKET_HEADER);
    print_padded_int(tcp_client.client, packet.cabinet, ProjectConfig::Protocol::CABINET_WIDTH);

    tcp_client.client.print("R");
    print_padded_int(tcp_client.client, packet.row, ProjectConfig::Protocol::ROW_WIDTH);

    tcp_client.client.print("C");
    print_padded_int(tcp_client.client, packet.column, ProjectConfig::Protocol::COLUMN_WIDTH);

    tcp_client.client.print("Q");
    print_padded_int(tcp_client.client, packet.quantity, ProjectConfig::Protocol::QUANTITY_WIDTH);

    tcp_client.client.print("L");
    print_padded_int(tcp_client.client, packet.color, ProjectConfig::Protocol::COLOR_WIDTH);

    tcp_client.client.print("M");
    print_padded_int(tcp_client.client, packet.command, ProjectConfig::Protocol::COMMAND_WIDTH);

    tcp_client.client.print("T");
    print_padded_int(tcp_client.client, packet.ret_status, ProjectConfig::Protocol::STATUS_WIDTH);

    tcp_client.client.print("N");
    print_padded_int(tcp_client.client, packet.transition, ProjectConfig::Protocol::TRANSITION_WIDTH);

    tcp_client.client.print("D");
    print_padded_int(tcp_client.client, packet.device, ProjectConfig::Protocol::DEVICE_WIDTH);

    tcp_client.client.print("S");
    print_padded_int(tcp_client.client, packet.sum, ProjectConfig::Protocol::CHECKSUM_WIDTH);

    tcp_client.client.println();

    // Success
    LOG_VERBOSE_F(CAT_TCP, "Packet sent to client - Cabinet:%d Row:%d Col:%d Qty:%d Color:%d Cmd:%d Status:%d Trans:%d Device:%d Sum:%d", 
                packet.cabinet, packet.row, packet.column, packet.quantity, packet.color,
                packet.command, packet.ret_status, packet.transition, packet.device, packet.sum);

    return 1;
}

bool mqtt_init()
{
    if (!ProjectConfig::Network::MQTT_ENABLED)
    {
        mqtt_info.connected = false;
        mqtt_info.last_error = "";
        return false;
    }

    mqtt_client.setServer(ProjectConfig::Network::MQTT_BROKER_IP, ProjectConfig::Network::MQTT_BROKER_PORT);
    if (mqtt_client.connect(mqtt_client_id.c_str(), ProjectConfig::Network::MQTT_USERNAME, ProjectConfig::Network::MQTT_PASSWORD)) 
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
    if (!ProjectConfig::Network::MQTT_ENABLED)
    {
        mqtt_info.connected = false;
        return;
    }

    if (!mqtt_client.connected()) 
    {
        uint32_t current_time = millis();
        
        // Check if enough time has passed since last reconnect attempt
        if (current_time - mqtt_info.last_reconnect_attempt >= ProjectConfig::Network::MQTT_RECONNECT_INTERVAL_MS)
        {
            LOG_WARN_MSG(CAT_MQTT, "Disconnected, attempting reconnect");
            mqtt_info.last_reconnect_attempt = current_time;

            if (mqtt_client.connect(mqtt_client_id.c_str(), ProjectConfig::Network::MQTT_USERNAME, ProjectConfig::Network::MQTT_PASSWORD)) 
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
        // If not enough time has passed, just mark as disconnected
        else
        {
            mqtt_info.connected = false;
        }
    }
    else
    {
        mqtt_info.connected = true;
    }
    mqtt_client.loop(); // Process incoming/outgoing MQTT messages
}

bool mqtt_publish_json(const char* level, const String& message, const char* topic)
{
    if (!ProjectConfig::Network::MQTT_ENABLED)
    {
        return false;
    }

    if (!mqtt_info.connected) 
    {
        LOG_ERROR_MSG(CAT_MQTT, "Cannot publish - not connected to broker");
        return false;
    }
    
    String json = "{";
    json += "\"device_id\":\"" + String(mqtt_device_id) + "\",";
    json += "\"level\":\"" + String(level) + "\",";
    json += "\"message\":\"" + message + "\"";
    json += "}";

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

bool publish_tcp_packet(const TcpPacket& packet, const char* type, const String& prefix)
{
    String msg = prefix +
        "Cabinet:" + String(packet.cabinet) +
        " Row:" + String(packet.row) +
        " Col:" + String(packet.column) +
        " Qty:" + String(packet.quantity) +
        " Color:" + String(packet.color) +
        " Cmd:" + String(packet.command) +
        " Status:" + String(packet.ret_status) +
        " Trans:" + String(packet.transition) +
        " Device:" + String(packet.device) +
        " Sum:" + String(packet.sum);

    return mqtt_publish_json(type, msg);
}