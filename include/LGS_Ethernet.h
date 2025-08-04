/*
 *  LGS_Ethernet.h  
 *  This library is used for communication between Arduino Opta and server using Ethernet protocol.
 *  
 *  Author: M.Teerachot
 *  Tested with Arduino Opta
 */

#ifndef LGS_ETHERNET_H
#define LGS_ETHERNET_H

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

// (0) Ethernet module initialization
#define CLIENT_NUM  2 // Old Ethernet module (W5100) supports up to 4 Clients.
extern IPAddress ip;
extern IPAddress subnet;
extern IPAddress gateway;
extern IPAddress dns;
extern EthernetServer server;
extern EthernetClient client;
extern bool ethernetInitialized;

// (1) Packet status (follow M01 in documentation)
#define PACK_FIRST_SUCCEED    1
#define PACK_SECOND_SUCCEED   2
#define PACK_NO_ACTION        3
#define PACK_IDLE             4
#define PACK_BUSY             5
#define PACK_FAIL             9

// (2) Packet parameter     
// CBT: Cabinet, ROW: Row, COL: Column, QTY: Quantity, CLR: Color, CMD: Command, RET: Return status, TRS: Transition number, DEV: Device
extern int CBT, ROW, COL, QTY, CLR, CMD, RET, DEV, SUM, SUM_CAL;
extern int CBT_IN, ROW_IN, COL_IN, QTY_IN, CLR_IN, CMD_IN, RET_IN, TRS_IN, DEV_IN, SUM_IN, SUM_CAL_IN;
extern uint8_t TRS[]; // Stores the transition number of each device.

// (3) Function Prototypes
void server_init();
int stringToInt(String string, int digit);
int digitLimiter(int num);
int getNumConnectedClient();
void newClient_Event();
void killClient_Event();
void clientUpdate();
void returnPacket(int cbt, int row, int col, int qty, int clr, int cmd, int ret, int trs, int dev);
int receivePacket();

#endif /* LGS_ETHERNET_H */
