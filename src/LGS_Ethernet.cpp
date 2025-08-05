
#include "LGS_Ethernet.h"

IPAddress ip(192, 168, 0, 59);  // Set your desired static IP address
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 0, 1);
IPAddress dns(8, 8, 8, 8);
EthernetServer server(2000);
EthernetClient client;

int CBT = 0, ROW = 0, COL = 0, QTY = 0, CLR = 0, CMD = 0, RET = 0, DEV = 0, SUM = 0, SUM_CAL = 0;
int CBT_IN = 0, ROW_IN = 0, COL_IN = 0, QTY_IN = 0, CLR_IN = 0, CMD_IN = 0, RET_IN = 0, TRS_IN = 0, DEV_IN = 0, SUM_IN = 0, SUM_CAL_IN = 0;

uint8_t TRS[9999];  // Stores the transition number of each device.
bool ethernetInitialized = false;

void server_init()
{
  ethernetInitialized = Ethernet.begin(ip, dns, gateway, subnet);
  if (ethernetInitialized)
  {
    if (Ethernet.linkStatus() == LinkOFF) 
    {
      Serial.println("Ethernet: cable is not connected.");
    }
    else 
    {
      // 0) Initialize Ethernet server
      server.begin();

      // 1) Show IP address
      Serial.print("Opta/IP: ");
      Serial.println(Ethernet.localIP());  
    
      // 2) Show MAC address
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
  else 
  {
     Serial.println("Ethernet: failed to begin.");
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

void clientUpdate()
{
  EthernetClient newClient = server.accept();

  // 1) If a new client is connected, assign it to the client variable.
  if (!client && newClient) 
  {
    client = newClient;
    Serial.println("Client: connected");
  }
  
  // 2) If the client is not connected, stop it.
  if (client && !client.connected()) 
  {
    client.stop();
    Serial.println("Client: disconnected");
  }  
}

void returnPacket(int cbt, int row, int col, int qty, int clr, int cmd, int ret, int trs, int dev)
{ 
  if (client && client.connected())
  {
    int sum = digitLimiter(cbt + row + col + qty + clr + cmd + ret + trs + dev); 

    client.print("B");    // Cabinet
    if (cbt < 10) client.print("0");  
    client.print(cbt);  

    client.print("R");    // Row
    if (row < 10) client.print("0");  
    client.print(row);  

    client.print("C");    // Column
    if (col < 10) client.print("0");  
    client.print(col);  

    client.print("Q");    // Quatity
    if (qty < 1000) client.print("0");  
    if (qty < 100)  client.print("0");  
    if (qty < 10)   client.print("0");  
    client.print(qty);  

    client.print("L");    // Color
    if (clr < 10) client.print("0");  
    client.print(clr);  

    client.print("M");    // Command
    if (cmd < 10) client.print("0");  
    client.print(cmd);  

    client.print("T");    // Return status
    if (ret < 10) client.print("0");  
    client.print(ret);

    client.print("N");    // Transition number
    if (trs < 10) client.print("0");  
    client.print(trs);  

    client.print("D");    // Device
    if (dev < 1000) client.print("0");  
    if (dev < 100)  client.print("0");  
    if (dev < 10)   client.print("0");  
    client.print(dev);  

    client.print("S");    // Summary check
    if (sum < 10) client.print("0");  
    client.print(sum);

    client.println();  
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
  // Example packet 2: B01R08C01Q0002L01M01T00N01D0000S15 (row 8, column 1, red on)
  // Example packet 3: B01R08C01Q0002L01M00T00N02D0000S15 (row 8, column 1, red off)
  // Example packet 4: B01R08C01Q0002L01M02T00N03D0000S18 (row 8, column 1, red idle)
  // Example packet 5: B01R00C00Q0000L01M09T00N05D0000S16 (hardware reset)

  // 1) Check if the client is connected and has data to read. 
  if (client && client.find('B')) 
  {
    CBT_IN = client.parseInt();
    ROW_IN = client.parseInt();
    COL_IN = client.parseInt();
    QTY_IN = client.parseInt();
    CLR_IN = client.parseInt();
    CMD_IN = client.parseInt();
    RET_IN = client.parseInt();
    TRS_IN = client.parseInt();
    DEV_IN = client.parseInt();
    SUM_IN = client.parseInt();
    SUM_CAL_IN = digitLimiter(CBT_IN + ROW_IN + COL_IN + QTY_IN + CLR_IN + CMD_IN + RET_IN + TRS_IN + DEV_IN); 

    // 2) Check if the received packet is valid.          
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

    // 3) Received packet for debugging.     
    // Serial.print("Cabinet:\t");     Serial.println(CBT_IN);
    // Serial.print("Row:\t\t");       Serial.println(ROW_IN);
    // Serial.print("Column:\t\t");    Serial.println(COL_IN); 
    // Serial.print("Qty:\t\t");       Serial.println(QTY_IN);
    // Serial.print("Color:\t\t");     Serial.println(CLR_IN);
    // Serial.print("Command:\t");     Serial.println(CMD_IN);
    // Serial.print("Return:\t\t");    Serial.println(RET_IN);
    // Serial.print("Transit:\t");     Serial.println(TRS_IN);
    // Serial.print("Device:\t\t");    Serial.println(DEV_IN);
    // Serial.print("Sum:\t\t");       Serial.println(SUM_IN);
    // Serial.print("Sum Cal:\t");     Serial.println(SUM_CAL_IN);
  }
  return ret;
}
