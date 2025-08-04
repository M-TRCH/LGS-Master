
#include "LGS_Ethernet.h"

IPAddress ip(192, 168, 0, 59);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 0, 1);
IPAddress dns(8, 8, 8, 8);
EthernetServer server(2000);
EthernetClient clients[CLIENT_NUM + 1];

int CBT = 0, ROW = 0, COL = 0, QTY = 0, CLR = 0, CMD = 0, RET = 0, DEV = 0, SUM = 0, SUM_CAL = 0;
int CBT_IN = 0, ROW_IN = 0, COL_IN = 0, QTY_IN = 0, CLR_IN = 0, CMD_IN = 0, RET_IN = 0, TRS_IN = 0, DEV_IN = 0, SUM_IN = 0, SUM_CAL_IN = 0;

uint8_t TRS[9999];
int prevClientConnected = 0;

void server_init()
{
  Ethernet.begin(ip, dns, gateway, subnet);
  if (Ethernet.linkStatus() == LinkOFF) 
  {
    Serial.println("Ethernet: cable is not connected.");
  }
  else 
  {
    // (0) open server
    server.begin();

    // (1) show ip address
    Serial.print("Opta/IP: ");
    Serial.println(Ethernet.localIP());  
  
    // (2) show mac address
    byte macBuffer[6];  
    Ethernet.MACAddress(macBuffer); 
    Serial.print("Opta/MAC: ");
    
    for (int octet = 5; octet >= 0; octet--) 
    {
      Serial.print(macBuffer[octet], HEX);
      if (octet > 0) 
      {
        Serial.print('-');
      }
    }
    Serial.println();
  }
}

int stringToInt(String string, int digit)
{
  int intRet = 0;
  int weight = 1;
  digit = constrain(digit, 1, 5);
  
  for (int i=digit-1; i>=0; i--)
  {
    String subString = string.substring(i, i+1);
         if (subString == "1")  intRet += (1 * weight);
    else if (subString == "2")  intRet += (2 * weight);
    else if (subString == "3")  intRet += (3 * weight);
    else if (subString == "4")  intRet += (4 * weight);
    else if (subString == "5")  intRet += (5 * weight);
    else if (subString == "6")  intRet += (6 * weight);
    else if (subString == "7")  intRet += (7 * weight);
    else if (subString == "8")  intRet += (8 * weight);
    else if (subString == "9")  intRet += (9 * weight); 
    weight *= 10;
  }
  return intRet;
}

int digitLimiter(int num) 
{
  String numStr = (String)num;  
  int strLen = numStr.length();
  int numCal =  stringToInt(numStr.substring(strLen-2, strLen), 2);
  if (num < 10) numCal = num; 
  return numCal;
}

int getNumConnectedClient()
{
  int numConnected = 0;
  for (byte i=0; i<CLIENT_NUM; i++) 
  {
    if (clients[i]) 
      numConnected++;
  }
  return numConnected;
}

void newClient_Event()
{
  // Welcoming new clients.
  EthernetClient newClient = server.accept();  

  if (newClient) 
  {
    if (getNumConnectedClient() >= CLIENT_NUM)
    {
      int cli = prevClientConnected + 1;
      if (cli >= CLIENT_NUM) cli = 0;
      
      clients[cli].stop();
      Serial.print("Client/"); 
      Serial.print(cli);
      Serial.println(": force stop");
    }
    
    for (byte i=0; i<CLIENT_NUM; i++) 
    {
      if (!clients[i]) 
      {
        prevClientConnected = i;
        Serial.print("Client/"); 
        Serial.print(i);
        Serial.println(": connected");
        clients[i] = newClient;    
        break;
      }
    }
  } 
}

void killClient_Event()
{
  // Cancelling service for inactive clients.
  for (byte i=0; i<CLIENT_NUM; i++) 
  {
    if (clients[i] && !clients[i].connected()) 
    {
      Serial.println("Client[" + String(i) + "]: disconnected"); 
      clients[i].stop();
    }
  }
}

void returnPacket(int cbt, int row, int col, int qty, int clr, int cmd, int ret, int trs, int dev)
{ 
  // (1) send return packet
  for (byte i=0; i<CLIENT_NUM; i++) 
  {
    if (clients[i].connected())
    {
      int sum = digitLimiter(cbt + row + col + qty + clr + cmd + ret + trs + dev); 
      
      clients[i].print("B");    // cabinet
      if (cbt < 10) 
        clients[i].print("0");  
      clients[i].print(cbt);  
      
      clients[i].print("R");    // row
      if (row < 10) 
        clients[i].print("0");  
      clients[i].print(row);  
      
      clients[i].print("C");    // column
      if (col < 10)
        clients[i].print("0");  
      clients[i].print(col);  

      clients[i].print("Q");    // quatity
      if (qty < 1000)
        clients[i].print("0");  
      if (qty < 100)
        clients[i].print("0");  
      if (qty < 10)
        clients[i].print("0");  
      clients[i].print(qty);  

      clients[i].print("L");    // color
      if (clr < 10)
        clients[i].print("0");  
      clients[i].print(clr);  

      clients[i].print("M");    // command
      if (cmd < 10)
        clients[i].print("0");  
      clients[i].print(cmd);  

      clients[i].print("T");    // status return
      if (ret < 10)
        clients[i].print("0");  
      clients[i].print(ret);

      clients[i].print("N");    // transition number
      if (trs < 10)
        clients[i].print("0");  
      clients[i].print(trs);  

      clients[i].print("D");    // device
      if (dev < 1000)
        clients[i].print("0");  
      if (dev < 100)
        clients[i].print("0");  
      if (dev < 10)
        clients[i].print("0");  
      clients[i].print(dev);  

      clients[i].print("S");    // summary check
      if (sum < 10)
        clients[i].print("0");  
      clients[i].print(sum);

      clients[i].println();  
    }
  }
}

int receivePacket()
{  
  int ret = 0;
  // Example packet: B03R01C02Q0098L01M03T00N01D4000S09
  // Header:            B
  // Cabinet:           03
  // Row:               01
  // Column:            02
  // Quantity:          0098
  // Color:             01
  // Command:           03 
  // Status Return:     00
  // Transition Number: 01
  // Device:            4520
  // Sum:               28
  //
  // Example packet 2: B03R11C22Q0001L02M30T00N02D3299S70
  // Example packet 3: B01R10C05Q0122L55M01T00N09D0999S02
  // Example packet 4: B01R10C05Q9122L55M01T00N08D0086S88
  // Example packet 5: B01R10C05Q9122L55M01T00N03D0004S01
  // Example packet 6: B01R00C01Q0002L01M01T00N04D4500S10  (ID=1, red on)
  // Example packet 7: B01R00C01Q0002L01M00T00N05D4500S10  (ID=1, red off)
 
  // (1) read packet
  for (byte i=0; i<CLIENT_NUM; i++) 
  {
    if (clients[i] && clients[i].find('B')) 
    {
      CBT_IN = clients[i].parseInt();
      ROW_IN = clients[i].parseInt();
      COL_IN = clients[i].parseInt();
      QTY_IN = clients[i].parseInt();
      CLR_IN = clients[i].parseInt();
      CMD_IN = clients[i].parseInt();
      RET_IN = clients[i].parseInt();
      TRS_IN = clients[i].parseInt();
      DEV_IN = clients[i].parseInt();
      SUM_IN = clients[i].parseInt();
      SUM_CAL_IN = digitLimiter(CBT_IN + ROW_IN + COL_IN + QTY_IN + CLR_IN + CMD_IN + RET_IN + TRS_IN + DEV_IN); 

      // (2) violations check
      if (SUM_IN == SUM_CAL_IN && TRS_IN != TRS[DEV_IN])
      {
        ret = 1;
        CBT = CBT_IN;
        ROW = ROW_IN;
        COL = COL_IN;
        QTY = QTY_IN;
        CLR = CLR_IN;
        CMD = CMD_IN;
        RET = RET_IN;
        DEV = DEV_IN;
        SUM = SUM_IN;
        TRS[DEV] = TRS_IN;
        returnPacket(CBT, ROW, COL, QTY, CLR, CMD, PACK_FIRST_SUCCEED, TRS[DEV], DEV);
      }

      // (3) debug
//      Serial.print("Cabinet:\t");     Serial.println(CBT_IN);
//      Serial.print("Row:\t\t");       Serial.println(ROW_IN);
//      Serial.print("Column:\t\t");    Serial.println(COL_IN); 
//      Serial.print("Qty:\t\t");       Serial.println(QTY_IN);
//      Serial.print("Color:\t\t");     Serial.println(CLR_IN);
//      Serial.print("Command:\t");     Serial.println(CMD_IN);
//      Serial.print("Return:\t\t");    Serial.println(RET_IN);
//      Serial.print("Transit:\t");     Serial.println(TRS_IN);
//      Serial.print("Device:\t\t");    Serial.println(DEV_IN);
//      Serial.print("Sum:\t\t");       Serial.println(SUM_IN);
//      Serial.print("Sum Cal:\t");     Serial.println(SUM_CAL_IN);
    }
  }
  return ret;
}
