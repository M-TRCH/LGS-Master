
#include "system.h"
#include "config.h"
#include "ethernet_utils.h"

/*
#define DEV_MODE_TIMEOUT      2000
#define WATCHDOG_TIMEOUT      30000  // 30 sec
// .3 Variables
unsigned long devModeTimer = millis();
bool devModeActive = false;
unsigned long kickWatchdogTimer = millis();

// (3) Functions
void RESET_Event(unsigned long preResetTime = 3000, unsigned long postResetTime = 1000)
{
    setInfo(5, 0, VERSION_DD, VERSION_MM, VERSION_YY); // white
    delay(preResetTime);
    PWR_RELAY2(LOW);
    PWR_RELAY4(LOW);      
    delay(postResetTime);
    NVIC_SystemReset();
}

void W_SW_Event()
{
    if (W_SW) 
    {
        delay(50);
        if (W_SW)
        {
            RESET_Event();
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

bool run()
{
    // 0) initialize variables
    bool getPacket = false;

    // 1) Get packet from client.
    if (receivePacket())  
    {
        getPacket = true;
        int ret = 0;

        // (2) Take action based on the command.
        #ifdef LGS_STANDARD
            // Invert bin location and convert to physical id.
            // from (1 to 8) ->  new (8 to 1)
            // Serial.println("(" + String(9-ROW) + (", ") + String(COL) + ")");
            switch (CMD)
            {
                case 0:
                    //  Set color off. 
                    ret = std_setColor(9-ROW, COL, CLR, false);
                    if (ret)    returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_SECOND_SUCCEED, TRS[DEV], DEV);   
                    else        returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV);   
                    break;

                case 1: 
                    //  Set color on.
                    ret = std_setColor(9-ROW, COL, CLR);
                    if (ret)    returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_SECOND_SUCCEED, TRS[DEV], DEV);   
                    else        returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV);   
                    break;

                case 2:
                    //  Check if the device is idle or busy.
                    //  If idle, return PACK_IDLE.
                    //  If busy, return PACK_BUSY.
                    //  If error, return PACK_FAIL.  
                    ret = std_requestStatus(9-ROW, COL, CLR);
                    if (ret == 1)       returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_IDLE, TRS[DEV], DEV);  
                    else if (ret == 2)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_BUSY, TRS[DEV], DEV);  
                    else                returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV); 
                    break;

                case 9:
                    //  Hardware reset.
                    RESET_Event();
                    break;
            }
        #endif

        #ifdef LGS_NARCOTIC
        // Convert bin location to physical id.
        // from (1 to 10) ->  new (9 to 0)
        // Serial.println("(" + String(10-ROW) + (", ") + String(COL) + ")");

            switch (CMD)
            {
                case 0: // off
                    ret = nct_setActive(10-ROW, COL);
                    if (ret == 1)       returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_NO_ACTION, TRS[DEV], DEV);   
                    else if (ret == 2)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_SECOND_SUCCEED, TRS[DEV], DEV);   
                    else                returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV);     
                    break;
                
                case 1: // on
                    ret = nct_setActive(10-ROW, COL, CLR, 50, QTY);  // brightness: 50 
                    if (ret)    returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_SECOND_SUCCEED, TRS[DEV], DEV);   
                    else        returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV);
                    break;

                case 2: // request status
                    ret = nct_requestStatus(10-ROW, COL);
                    if (ret == 1)       returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_IDLE, TRS[DEV], DEV);  
                    else if (ret == 2)  returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_BUSY, TRS[DEV], DEV);  
                    else                returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FAIL, TRS[DEV], DEV); 
                    break;

                case 9: // hardware reset
                    RESET_Event();
                break;
            }
        #endif
    }
    return getPacket;
}

*/

void setup() 
{
    #ifdef SYSTEM_H
        system_init();
    #endif

    #ifdef CONFIG_H
        config_init();
    #endif

    #ifdef ETHERNET_UTILS_H
        ethernet_init();
    #endif

    // // .2 Subsystem initialize
    // #ifdef LGS_MASTER_H
    //     commu_init();
    // #endif

    // #ifdef LGS_ETHERNET_H
    //     server_init();
    // #endif

    // // .4 Development mode
    // devModeTimer = millis();
    // while (W_SW || R_SW || G_SW || B_SW || Y_SW)
    // {
    //     delay(10);
    //     if (millis()-devModeTimer >= DEV_MODE_TIMEOUT)
    //     {
    //         devModeActive = true;
    //         setInfo(1, 0, VERSION_DD, VERSION_MM, VERSION_YY);  delay(1000);  // red
    //         setInfo(2, 0, VERSION_DD, VERSION_MM, VERSION_YY);  delay(1000);  // green
    //         setInfo(3, 0, VERSION_DD, VERSION_MM, VERSION_YY);  delay(1000);  // blue
    //         setInfo(4, 0, VERSION_DD, VERSION_MM, VERSION_YY);  delay(1000);  // yellow
    //         setInfo(5, 0, VERSION_DD, VERSION_MM, VERSION_YY);  delay(1000);  // white
    //         setInfo(0, 0, VERSION_DD, VERSION_MM, VERSION_YY);  
    //         Serial.println("Opta/status: develop mode");
    //         break;   
    //     }
    // }
  
    // // .5 Test functions
    // #ifdef TEST_FUNCTION
    //     #ifdef LGS_STANDARD
    //         std_moduleCheck();
    //     #endif 
    //     #ifdef LGS_NARCOTIC
    //         // nct_moduleCheck();
    //         nct_allUnlock();
    //     #endif
    // #endif
 
    // // .6 Second start up 
    // if (!devModeActive) 
    // {
    //     setInfo(2, 0, VERSION_DD, VERSION_MM, VERSION_YY);  // green
    //     // Enable watchdog timer
    //     mbed::Watchdog::get_instance().start(WATCHDOG_TIMEOUT);
    // }
}

void loop() 
{
    // panel_switch_debug();
    
    // PRINT(DEBUG_BASIC, F("IP Address: "));
    // PRINT(DEBUG_BASIC, ipToString(&device_info.ip_address));
    // PRINT(DEBUG_BASIC, F("\nFirmware Version: "));
    // PRINT(DEBUG_BASIC, firmwareVersionToString(&device_info.firmware_version));
    // PRINT(DEBUG_BASIC, F("\n"));
    // delay(1000);

    // .2 Ethernet
    // clientUpdate();

    // .3 Development mode
    // if (devModeActive)
    // {
    //     R_SW_Event();
    //     G_SW_Event();
    //     B_SW_Event();
    //     Y_SW_Event();
    // }
  
    // .4 Run main function
    // else
    // {    
    //     // If the client is connected, set the status to idle.
    //     if (cilentAlready && cilentAlreadyFirstCycle)
    //     {
    //         cilentAlreadyFirstCycle = false;
    //         setInfo(1, 0, VERSION_DD, VERSION_MM, VERSION_YY);  // red 
    //     }

    //     // If the client is not connected, set the status to busy.
    //     else if (!cilentAlready && !cilentAlreadyFirstCycle)
    //     {
    //         cilentAlreadyFirstCycle = true;
    //         setInfo(2, 0, VERSION_DD, VERSION_MM, VERSION_YY);  // green
    //     }

    //     // If the packet is received, forced stop the client.
    //     if (run())  
    //     {
    //         client.stop();
    //         cilentAlready = false;
    //         cilentAlreadyFirstCycle = false;
    //         Serial.println(clientInfo + " -> disconnected (force)");
    //     }

    //     // If the client is connected for too long, force stop the client.
    //     if (millis() - last_client_connected_time >= 3000 && cilentAlready)
    //     {
    //         client.stop();
    //         cilentAlready = false;
    //         cilentAlreadyFirstCycle = false;
    //         Serial.println(clientInfo + " -> disconnected (timeout)");
    //     }
    // }
    
    // // .5 Reset watchdog timer
    // if (millis() - kickWatchdogTimer >= WATCHDOG_TIMEOUT / 4)
    // {
    //     kickWatchdogTimer = millis();
    //     mbed::Watchdog::get_instance().kick();
    // }

    // // .6 Reset switch
    // W_SW_Event();
}

