
 /*
 *  LGSbus.h 
 *  This library is used for communication between Arduino Opta and other devices using RS485 protocol.
 * 
 *  Author: M.Teerachot
 *  Tested with Arduino Opta
 */

#ifndef LGSBUS_H
#define LGSBUS_H

#include <Arduino.h>
#include <ArduinoRS485.h>

class LGSbus
{
  #define DATA_BUF_SIZE     30  
  #define DATA_SUBBUF_SIZE  16  
  #define READ_TYPE         0
  #define WRITE_TYPE        1
  #define RETURN_TYPE       2
      
  private:
    uint16_t ID;
    int DATA_BUF[DATA_BUF_SIZE][DATA_SUBBUF_SIZE];
    int LAST_ADDR;
    int retDelayTime = 5;
    
  public:
    const uint8_t ID_MASTER = -1; 

    /* config */
    LGSbus(); 
    ~LGSbus();
    void begin(int id, RS485Class* serial, int baud, int timeout=50);
    void setReturnDelayTime(int t);
    
    /* general */
    int send(int id, int type, int len, int addr, int timeout, int cnt_max);
    int readData(int addr, int index);
    void writeData(int addr, int index, int data);
    
    /* master */
    int write(int id, int len, int addr, int timeout=500, int cnt_max=1);
    int read(int id, int addr, int timeout=500, int cnt_max=1);
    
    /* slave */
    int poll();
    int readLastAddress();
    
  protected:
    RS485Class *SR;
};

#endif /* LGSBUS_H */