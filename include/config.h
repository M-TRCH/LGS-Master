
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "system.h"

/* @brief Initialize configuration settings
*/
void config_init();

// Struct for storing IP address
struct IPAddress_t 
{
    uint8_t ip1;
    uint8_t ip2;
    uint8_t ip3;
    uint8_t ip4;
};

// Struct for storing firmware version
struct FirmwareVersion_t 
{
    uint8_t day;    // day (1-31)
    uint8_t month;  // month (1-12)
    uint16_t year;  // year (e.g. 2025)
};

// Struct for storing all device information
struct DeviceInfo_t 
{
    IPAddress_t ip_address;
    FirmwareVersion_t firmware_version;
};

// Global variable to store device information
extern DeviceInfo_t device_info;

/* @brief Set the IP address of the device
 * @param info Pointer to the DeviceInfo_t structure
 * @param ip1 First octet of the IP address
 * @param ip2 Second octet of the IP address
 * @param ip3 Third octet of the IP address
 * @param ip4 Fourth octet of the IP address
 */
void setIPAddress(DeviceInfo_t* info, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4);

/*
 * @brief Set the firmware version of the device
 * @param info Pointer to the DeviceInfo_t structure
 * @param day Day of the firmware version (1-31)
 * @param month Month of the firmware version (1-12)
 * @param year Year of the firmware version (e.g. 2025)
 */
void setFirmwareVersion(DeviceInfo_t* info, uint8_t day, uint8_t month, uint16_t year);

/* @brief Convert IP address to String
 * @param ip Pointer to the IPAddress_t structure
 */
String ipToString(const IPAddress_t* ip);

/* @brief Convert firmware version to String
 * @param version Pointer to the FirmwareVersion_t structure
 */
String firmwareVersionToString(const FirmwareVersion_t* version);

/* @brief Print device information
 * @param info Pointer to the DeviceInfo_t structure
 */
void printDeviceInfo(const DeviceInfo_t* info);

#endif // CONFIG_H
