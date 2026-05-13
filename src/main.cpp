#include <ArduinoRS485.h> 
#include <ArduinoModbus.h> 

ModbusRTUClientClass mdb_client; 
void setup() 
{
  Serial.begin(9600); 
  RS485.setDelays(10000, 1000); // Set pre and post transmission delays to 10ms and 1ms respectively
  
  if (!mdb_client.begin(RS485, 9600, SERIAL_8N1)) 
  {
    Serial.println("Failed to start Modbus RTU Client!");
    return;
  }
  else
  {
    Serial.println("Modbus RTU Client started successfully.");
  }
}   

void readCoilValues(int  slave_id = 61, int start_address = 1001, int quantity = 28) 
{
  Serial.print("[S");
  Serial.print(slave_id);
  Serial.print("] ");

  if (!mdb_client.requestFrom(slave_id, COILS, start_address, quantity)) {
    Serial.println(mdb_client.lastError());
  } else {
    Serial.print(start_address);
    Serial.print("-");
    Serial.print(start_address + quantity - 1);
    Serial.print(": ");
    while (mdb_client.available()) {
      Serial.print(mdb_client.read());
      Serial.print(" ");
    }
    Serial.println();
  }
}

void loop() 
{
  for (int id = 61; id <= 68; id++) 
  {
    readCoilValues(id); // Read coil values from the Modbus slave device
    delay(50);
  }
}


