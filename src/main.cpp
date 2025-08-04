
#include <Arduino.h>
#include "LGS_Master.h"
#include "LGS_Ethernet.h"

/* (0) System */
#define GENERAL_BAUD      115200
#define PWR_RELAY2_PIN    D1
#define PWR_RELAY4_PIN    D3
#define PWR_RELAY2(s)     digitalWrite(PWR_RELAY2_PIN, s);
#define PWR_RELAY4(s)     digitalWrite(PWR_RELAY4_PIN, s);
#define WAIT_MODULE_TIME  5000
// Development mode
#define unlockTimeout     2000
unsigned long unlockTimer = millis();
bool devMode = false;

/* (1) Operate */
//#define TEST_FUNC
#define LGS_STANDARD
//#define LGS_NARCOTIC

/* (2) Console */
#define W_SW  digitalRead(A0)
#define R_SW  digitalRead(A1)
#define G_SW  digitalRead(A2)
#define B_SW  digitalRead(A3)
#define Y_SW  digitalRead(A4)

/* (3) Functions */
void softwareReset()
{
  setInfo(5, 0, VERSION_DD, VERSION_MM, VERSION_YY);
  delay(3000);
  PWR_RELAY2(LOW);
  PWR_RELAY4(LOW);      
  delay(1000);
  NVIC_SystemReset();
}

void W_SW_Event()
{
  if (W_SW) 
  {
    delay(50);
    if (W_SW)
    {
      softwareReset();
    }
  }  
}

void R_SW_Event()
{
  if (R_SW) 
  {
    delay(50);
    if (R_SW)
    {
      setInfo(1, 1, VERSION_DD, VERSION_MM, VERSION_YY);
      
#ifdef LGS_STANDARD
      for (int cl=1; cl<=4; cl++)
      {
        for (int r=1; r<=8; r++)
        {
          for (int c=1; c<=8; c++)
          {
            std_setColor(r, c, cl);
          }
        }
      }
#endif

#ifdef LGS_NARCOTIC
  for (int c=1; c<=8; c++)
  {
    for (int r=0; r<=9; r++)  
    {
      Serial.print("[" + String(r) + ", " + String(c) + "]: ");
      Serial.println(nct_setActive(r, c, 1, 50, 0));  
      delay(1500);
    }
    for (int r=0; r<=9; r++)  
    {
      Serial.print("[" + String(r) + ", " + String(c) + "]: ");
      Serial.println(nct_setActive(r, c));  
      delay(1500);
    }
  }
#endif

      setInfo(0, 0, VERSION_DD, VERSION_MM, VERSION_YY);
    }
  }
}

void G_SW_Event()
{
  if (G_SW) 
  {
    delay(50);
    if (G_SW)
    {
      setInfo(2, 2, VERSION_DD, VERSION_MM, VERSION_YY);
      
#ifdef LGS_STANDARD
      for (int cl=1; cl<=4; cl++)
      {
        for (int r=1; r<=8; r++)
        {
          for (int c=1; c<=8; c++)
          {
            std_setColor(r, c, cl, false);
          }
        }
      }
#endif

#ifdef LGS_NARCOTIC
  delay(3000);
#endif

      setInfo(0, 0, VERSION_DD, VERSION_MM, VERSION_YY);
    }
  }
}

void B_SW_Event()
{
  if (B_SW) 
  {
    delay(50);
    if (B_SW)
    {
      setInfo(3, 3, VERSION_DD, VERSION_MM, VERSION_YY);
      
#ifdef LGS_STANDARD
      delay(3000);
#endif 

#ifdef LGS_NARCOTIC
      nct_allUnlock();
#endif

      setInfo(0, 0, VERSION_DD, VERSION_MM, VERSION_YY);
    }
  }
}

void Y_SW_Event()
{
  if (Y_SW) 
  {
    delay(50);
    if (Y_SW)
    {
      setInfo(4, 4, VERSION_DD, VERSION_MM, VERSION_YY);
    
#ifdef LGS_STANDARD
      delay(3000);
#endif 

#ifdef LGS_NARCOTIC
      nct_allLock();
#endif

      setInfo(0, 0, VERSION_DD, VERSION_MM, VERSION_YY);
    }
  }
}

void run()
{
  // (1) get packet
  if (receivePacket())  
  {
    int ret = 0;
    
    // (2) Take actions
#ifdef LGS_STANDARD
    // Convert bin location to physical id.
    // from (1 to 8) ->  new (8 to 1)
//    Serial.println("(" + String(9-ROW) + (", ") + String(COL) + ")");

    switch (CMD)
    {
      case 0: // off
        ret = std_setColor(9-ROW, COL, CLR, false);
        if (ret)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_SECOND_SUCCEED, TRS[DEV], DEV);   
        else      returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV);   
        break;
      
      case 1: // on 
        ret = std_setColor(9-ROW, COL, CLR);
        if (ret)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_SECOND_SUCCEED, TRS[DEV], DEV);   
        else      returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV);   
        break;

      case 2: // request status
        ret = std_requestStatus(9-ROW, COL, CLR);
             if (ret == 1)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_IDLE, TRS[DEV], DEV);  
        else if (ret == 2)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_BUSY, TRS[DEV], DEV);  
        else                returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV); 
        break;

      case 9: // hardware reset
        softwareReset();
        break;
    }
   
#endif

#ifdef LGS_NARCOTIC
    // Convert bin location to physical id.
    // from (1 to 10) ->  new (9 to 0)
//    Serial.println("(" + String(10-ROW) + (", ") + String(COL) + ")");

    switch (CMD)
    {
      case 0: // off
        ret = nct_setActive(10-ROW, COL);
             if (ret == 1)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_NO_ACTION, TRS[DEV], DEV);   
        else if (ret == 2)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_SECOND_SUCCEED, TRS[DEV], DEV);   
        else                returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV);     
        break;
      
      case 1: // on
        ret = nct_setActive(10-ROW, COL, CLR, 50, QTY);  // brightness: 50 
        if (ret)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_SECOND_SUCCEED, TRS[DEV], DEV);   
        else      returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV);
        break;

      case 2: // request status
        ret = nct_requestStatus(10-ROW, COL);
             if (ret == 1)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_IDLE, TRS[DEV], DEV);  
        else if (ret == 2)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_BUSY, TRS[DEV], DEV);  
        else                returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV); 
        break;

      case 9: // hardware reset
        softwareReset();
        break;
    }
#endif
  
  }
}

void setup() 
{
    // .1 System initialize
    Serial.begin(GENERAL_BAUD);
    pinMode(PWR_RELAY2_PIN, OUTPUT);
    pinMode(PWR_RELAY4_PIN, OUTPUT);
    PWR_RELAY2(HIGH);
    PWR_RELAY4(HIGH);

    // .2 Subsystem initialize
#ifdef LGS_MASTER_H
    commu_init();
#endif

#ifdef LGS_ETHERNET_H
    server_init();
#endif

  // .3 Start up
  delay(WAIT_MODULE_TIME);
  setInfo(0, 0, VERSION_DD, VERSION_MM, VERSION_YY);
  Serial.println("Opta/status: started");
  Serial.println("Code/version: " + String(VERSION_DD) + "/" + String(VERSION_MM) + "/" + String(VERSION_YY));

  // .4 Unlock development mode
  unlockTimer = millis();
  while (W_SW || R_SW || G_SW || B_SW || Y_SW)
  {
    delay(10);
    if (millis()-unlockTimer >= unlockTimeout)
    {
      devMode = true;
      setInfo(1, 0, VERSION_DD, VERSION_MM, VERSION_YY);  delay(1000);  // red
      setInfo(2, 0, VERSION_DD, VERSION_MM, VERSION_YY);  delay(1000);  // green
      setInfo(3, 0, VERSION_DD, VERSION_MM, VERSION_YY);  delay(1000);  // blue
      setInfo(4, 0, VERSION_DD, VERSION_MM, VERSION_YY);  delay(1000);  // yellow
      setInfo(5, 0, VERSION_DD, VERSION_MM, VERSION_YY);  delay(1000);  // white
      setInfo(0, 0, VERSION_DD, VERSION_MM, VERSION_YY);  
      Serial.println("Opta/status: develop mode");
      break;   
    }
  }
  
  // .5 Test functions
#ifdef TEST_FUNC
  #ifdef LGS_STANDARD
    std_moduleCheck();
  #endif
  
  #ifdef LGS_NARCOTIC
//    nct_moduleCheck();
    nct_allUnlock();
  #endif
#endif
 
  while(0)
  {

  }
//  while(1);
}

void loop() 
{
  // .1 Test functions
//  Serial.print(W_SW);
//  Serial.print(R_SW);
//  Serial.print(G_SW);
//  Serial.print(B_SW);
//  Serial.println(Y_SW);
  
  // .2 Ethernet
  newClient_Event();
  killClient_Event();

  // .3 Run main function
  run();
  
  // .4 Development mode
  if (devMode)
  {
    R_SW_Event();
    G_SW_Event();
    B_SW_Event();
    Y_SW_Event();
  }
  W_SW_Event();
}

