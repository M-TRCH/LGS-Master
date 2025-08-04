/*
 *  LGS_Master.h
 *  This library is used for communication between Arduino Opta and LGS modules using RS485 protocol.
 *  
 *  Author: M.Teerachot
 *  Tested with Arduino Opta
 */

#ifndef LGS_MASTER_H
#define LGS_MASTER_H

#include <Arduino.h>
#include "LGSbus.h"

// (0) System config    
#define RS485_BAUDRATE  9600
#define RS485_TIMEOUT   50
#define VERSION_DD      4
#define VERSION_MM      8
#define VERSION_YY      25

// (1) Define LGS protocol objects
extern LGSbus bus;

// (2) Refer to LGS8_COMMU.h
#define ADDR_GREET      0
#define ADDR_CL_LED1    1
#define ADDR_CL_LED2    2
#define ADDR_CL_LED3    3
#define ADDR_CL_LED4    4
#define ADDR_CL_LED5    5
#define ADDR_CL_LED6    6
#define ADDR_CL_LED7    7
#define ADDR_CL_LED8    8
#define ADDR_CL_LED12   9
#define ADDR_CL_LED34   10
#define ADDR_CL_LED56   11
#define ADDR_CL_LED78   12

// (3) Prototypes functions
// .1 Initialize communication
void commu_init();

// .2 Common functions
void setInfo(int cl, int ch, int dd, int mm, int yy);
int moduleCheck(int row, int col);

// .3 LGS standard functions
void std_moduleCheck();
int std_setColor(int id, int addr, int r, int g, int b, float f = 1.0);
int std_setColor(int row, int col, int cl, bool state = true);
int std_requestStatus(int row, int col, int cl_in);

// .4 LGS narcotic functions
void nct_moduleCheck();
int nct_setActive(int row, int col, int cl = 0, int bn = 0, int num = 0);
void nct_allUnlock();
void nct_allLock();
int nct_requestStatus(int row, int col);

#endif /* LGS_MASTER_H */
