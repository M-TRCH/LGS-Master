#include "config.h"

// Global variable to store device information
DeviceInfo_t device_info;

void setIPAddress(DeviceInfo_t* info, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4) 
{
    info->ip_address.ip1 = ip1;
    info->ip_address.ip2 = ip2;
    info->ip_address.ip3 = ip3;
    info->ip_address.ip4 = ip4;
}

void setFirmwareVersion(DeviceInfo_t* info, uint8_t day, uint8_t month, uint16_t year) 
{
    info->firmware_version.day = day;
    info->firmware_version.month = month;
    info->firmware_version.year = year;
}

String ipToString(const IPAddress_t* ip) 
{
    String result = String(ip->ip1) + "." + 
                   String(ip->ip2) + "." + 
                   String(ip->ip3) + "." + 
                   String(ip->ip4);
    return result;
}

String firmwareVersionToString(const FirmwareVersion_t* version) 
{
    String day = (version->day < 10) ? "0" + String(version->day) : String(version->day);
    String month = (version->month < 10) ? "0" + String(version->month) : String(version->month);
    return day + "/" + month + "/" + String(version->year);
}