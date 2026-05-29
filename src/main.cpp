#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoRS485.h>

// == =============== Hardware Configuration =================
#define Module_Relay_PIN  D0
#define LED_Relay_PIN     D1
#define SW_R_PIN          A0
#define SW_G_PIN          A1
#define SW_B_PIN          A2
#define SW_Y_PIN          A3
#define SW_W_PIN          A4

// ================= ตั้งค่า Network =================
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 178); 
EthernetServer tcpServer(502);EthernetClient client;   // Global — คงค่าระหว่าง loop() iterations
// ================= ค่าคงที่ =================
#define RS485_BAUD         9600
#define MODBUS_TCP_PORT    502
#define TCP_BUF_SIZE       256
#define RTU_BUF_SIZE       256
#define MBAP_HEADER_LEN    6
#define TIMEOUT_FIRST_BYTE 300UL   // ms รอไบต์แรกจาก RS485
#define TIMEOUT_INTER_BYTE 20UL    // ms รอระหว่างไบต์ (frame gap)

// ================= Debug Helper =================
void printHex(const char* label, const uint8_t* buf, int len) {
  Serial.print("[DBG] ");
  Serial.print(label);
  Serial.print(" (");
  Serial.print(len);
  Serial.print(" bytes): ");
  for (int i = 0; i < len; i++) {
    if (buf[i] < 0x10) Serial.print("0");
    Serial.print(buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

// ================= CRC16 Modbus =================
uint16_t calculateCRC(const uint8_t* buf, int len) {
  uint16_t crc = 0xFFFF;
  for (int i = 0; i < len; i++) {
    crc ^= (uint16_t)buf[i];
    for (int b = 8; b != 0; b--) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

bool verifyCRC(const uint8_t* buf, int len) {
  if (len < 3) return false;
  uint16_t calc = calculateCRC(buf, len - 2);
  uint16_t recv = (uint16_t)buf[len - 2] | ((uint16_t)buf[len - 1] << 8);
  return calc == recv;
}

void hardwareReset() {
  Serial.println("[SYS] Performing hardware reset...");
  digitalWrite(Module_Relay_PIN, LOW);  // ตัดไฟ
  digitalWrite(LED_Relay_PIN, LOW);  // ตัดไฟ
  delay(3000);
  Serial.println("[SYS] Reset complete.");
  NVIC_SystemReset();  // เรียกรีเซ็ตระบบ
}

// ================= Setup =================
void setup() 
{
  Serial.begin(9600);
  delay(500);
  Serial.println("========================================");
  Serial.println("  Transparent Modbus TCP-RTU Gateway");
  Serial.println("  Board: Arduino Opta");
  Serial.println("========================================");
  pinMode(Module_Relay_PIN, OUTPUT);  digitalWrite(Module_Relay_PIN, HIGH);   // Enable module power 
  pinMode(LED_Relay_PIN, OUTPUT);     digitalWrite(LED_Relay_PIN, HIGH);      // Enable LED for status indication  

  // 1. ตั้งค่า RS485
  Serial.println("[INIT] Configuring RS485...");
  RS485.setDelays(10000, 1000);   // pre-TX delay 10ms, post-TX delay 1ms (us)
  RS485.begin(RS485_BAUD);
  RS485.receive();                 // เริ่มต้นในโหมด RX
  Serial.print("[INIT] RS485 ready @ ");
  Serial.print(RS485_BAUD);
  Serial.println(" baud");

  // 2. ตั้งค่า Ethernet
  Serial.println("[INIT] Starting Ethernet...");
  Ethernet.begin(mac, ip);
  Serial.print("[INIT] IP Address: ");
  Serial.println(Ethernet.localIP());

  // 3. เริ่ม TCP Server
  tcpServer.begin();
  Serial.print("[INIT] Modbus TCP Server listening on port ");
  Serial.println(MODBUS_TCP_PORT);
  Serial.println("[INIT] Gateway ONLINE — waiting for connections...");
  Serial.println("========================================");

  // while (1)
  // {
  //   Serial.print(digitalRead(SW_R_PIN));
  //   Serial.print(digitalRead(SW_G_PIN));
  //   Serial.print(digitalRead(SW_B_PIN));
  //   Serial.print(digitalRead(SW_Y_PIN));
  //   Serial.println(digitalRead(SW_W_PIN));
  //   delay(500);
  // }
}

// ================= Loop =================
void loop() 
{
  // check reset button
  if (digitalRead(SW_W_PIN) == HIGH) 
  {
    delay(50); // debounce
    if (digitalRead(SW_W_PIN) == HIGH) 
    {
      hardwareReset();
      Serial.println("[SW] White button pressed.");
    }
  }

  // ---- รับ connection ใหม่ (ไม่ขึ้นกับว่ามีข้อมูลหรือยัง) ----
  EthernetClient newClient = tcpServer.accept();
  if (newClient) {
    if (client) {
      // มี client อยู่แล้ว — ปฏิเสธการเชื่อมต่อใหม่
      Serial.print("[NET] New connection from ");
      Serial.print(newClient.remoteIP());
      Serial.println(" refused — already serving a client.");
      newClient.stop();
    } else {
      client = newClient;
      Serial.print("[NET] Client connected: ");
      Serial.print(client.remoteIP());
      Serial.print(":");
      Serial.println(client.remotePort());
    }
  }

  // ---- ถ้าไม่มี client ใดๆ ก็รอต่อไป ----
  if (!client) return;

  // ---- ตรวจสอบว่า client ยังเชื่อมต่ออยู่ ----
  if (!client.connected()) {
    Serial.println("[NET] Client disconnected.");
    client.stop();
    return;
  }

  // ---- รอจนได้ MBAP Header ครบ 6 bytes ----
  if (client.available() < MBAP_HEADER_LEN) return;

  // ---- ขั้นที่ 1: อ่าน MBAP Header (6 bytes) ก่อน เพื่อรู้ความยาว Payload ----
  uint8_t tcp_buf[TCP_BUF_SIZE];
  int n = client.read(tcp_buf, MBAP_HEADER_LEN);
  if (n != MBAP_HEADER_LEN) {
    Serial.println("[ERR] Failed to read MBAP header.");
    return;
  }

  uint16_t transId = ((uint16_t)tcp_buf[0] << 8) | tcp_buf[1];
  uint16_t protoId = ((uint16_t)tcp_buf[2] << 8) | tcp_buf[3];
  uint16_t mbapLen = ((uint16_t)tcp_buf[4] << 8) | tcp_buf[5];

  Serial.println("----------------------------------------");
  Serial.print("[TCP] RX from ");
  Serial.print(client.remoteIP());
  Serial.print(":");
  Serial.println(client.remotePort());
  Serial.print("[TCP] Transaction ID : 0x"); Serial.println(transId, HEX);
  Serial.print("[TCP] Protocol ID    : 0x"); Serial.println(protoId, HEX);
  Serial.print("[TCP] MBAP Length    : ");   Serial.println(mbapLen);

  if (protoId != 0) {
    Serial.println("[ERR] Protocol ID != 0 — not Modbus TCP, discarding.");
    while (client.available()) client.read();  // flush
    return;
  }

  if (mbapLen < 2 || mbapLen > (TCP_BUF_SIZE - MBAP_HEADER_LEN)) {
    Serial.print("[ERR] MBAP Length out of range: ");
    Serial.println(mbapLen);
    while (client.available()) client.read();  // flush
    return;
  }

  // ---- รอให้ Payload ครบตาม mbapLen (TCP อาจส่งมาแบบ fragment) ----
  unsigned long t_tcp = millis();
  while (client.available() < (int)mbapLen) {
    if (millis() - t_tcp > 100UL) {
      Serial.print("[ERR] TCP payload timeout — got ");
      Serial.print(client.available());
      Serial.print("/");
      Serial.print(mbapLen);
      Serial.println(" bytes. Discarding.");
      while (client.available()) client.read();  // flush incomplete frame
      return;
    }
  }

  // ---- อ่าน Payload ครบในครั้งเดียว ----
  int payload_n = client.read(tcp_buf + MBAP_HEADER_LEN, (int)mbapLen);

  // Bug fix #5: ตรวจสอบว่าอ่านได้ครบตาม mbapLen จริงๆ
  // (อาจอ่านได้ไม่ครบถ้า client disconnect กลางคัน)
  if (payload_n != (int)mbapLen) {
    Serial.print("[ERR] Payload read mismatch: expected ");
    Serial.print(mbapLen);
    Serial.print(" got ");
    Serial.println(payload_n);
    while (client.available()) client.read();  // flush
    return;
  }

  int tcp_len = MBAP_HEADER_LEN + payload_n;
  printHex("TCP RX", tcp_buf, tcp_len);

  uint8_t unitId   = tcp_buf[6];
  uint8_t funcCode = tcp_buf[7];
  Serial.print("[TCP] Unit (Slave) ID: ");   Serial.println(unitId);
  Serial.print("[TCP] Function Code  : 0x"); Serial.println(funcCode, HEX);

  // ---- ขั้นที่ 2: สร้าง RTU Frame (ตัด MBAP + เพิ่ม CRC) ----
  // rtu_len = mbapLen (Unit ID + PDU) — ใช้ค่าจาก header ไม่ใช้ tcp_len
  int rtu_len = (int)mbapLen;
  if (rtu_len < 2 || rtu_len > (RTU_BUF_SIZE - 2)) {
    Serial.print("[ERR] RTU PDU length out of range: ");
    Serial.println(rtu_len);
    return;
  }

  uint8_t rtu_buf[RTU_BUF_SIZE];
  memcpy(rtu_buf, tcp_buf + MBAP_HEADER_LEN, rtu_len);

  uint16_t crc = calculateCRC(rtu_buf, rtu_len);
  rtu_buf[rtu_len]     = crc & 0xFF;         // CRC Low byte
  rtu_buf[rtu_len + 1] = (crc >> 8) & 0xFF;  // CRC High byte

  printHex("RTU TX", rtu_buf, rtu_len + 2);
  Serial.print("[RTU] CRC = 0x"); Serial.println(crc, HEX);

  // ---- ขั้นที่ 3: ส่งลงสาย RS485 ----
  // Flush RX buffer ก่อนส่ง เพื่อล้างข้อมูลค้างจาก transaction ก่อนหน้า
  // Bug fix #3: จำกัด loop เพื่อป้องกัน infinite loop ถ้า RS485 bus ส่งข้อมูลต่อเนื่อง
  int pre_flush = 0;
  while (RS485.available() && pre_flush < RTU_BUF_SIZE) { RS485.read(); pre_flush++; }
  if (pre_flush > 0) {
    Serial.print("[RS485] Pre-TX flush: discarded ");
    Serial.print(pre_flush);
    Serial.println(" stale bytes.");
  }

  int tx_frame_size = rtu_len + 2;  // จำนวน bytes ที่เราส่งออกไปจริงๆ (รวม CRC)

  Serial.println("[RS485] Switching to TX mode...");
  RS485.beginTransmission();
  RS485.write(rtu_buf, tx_frame_size);
  RS485.endTransmission();
  Serial.println("[RS485] Frame sent. Switching to RX mode...");
  RS485.receive();   // << สำคัญมาก: ต้องเรียกเพื่อเปิด RX หลัง TX เสร็จ

  // ---- ขั้นที่ 4: รอรับคำตอบจาก Slave ----
  uint8_t rx_buf[RTU_BUF_SIZE];
  int rx_idx = 0;
  bool receiving = false;
  unsigned long t_start = millis();

  Serial.print("[RS485] Waiting for response (timeout=");
  Serial.print(TIMEOUT_FIRST_BYTE);
  Serial.println("ms)...");

  while (true) {
    if (RS485.available()) {
      uint8_t b = (uint8_t)RS485.read();  // อ่านออกเสมอเพื่อไม่ให้ UART buffer เต็ม
      if (rx_idx < RTU_BUF_SIZE) {
        rx_buf[rx_idx++] = b;
        t_start = millis();   // Bug fix #2: reset timer เฉพาะเมื่อ buffer ยังรับได้
        receiving = true;
      }
      // ถ้า buffer เต็ม: อ่านทิ้ง ไม่ reset timer → ให้ timeout ทำงานได้
    }

    unsigned long elapsed = millis() - t_start;
    if (!receiving && elapsed > TIMEOUT_FIRST_BYTE) {
      Serial.println("[RS485] Timeout — no response from slave.");
      break;
    }
    if (receiving && elapsed > TIMEOUT_INTER_BYTE) {
      Serial.println("[RS485] Frame gap detected — end of frame.");
      break;
    }
  }

  Serial.print("[RS485] Received ");
  Serial.print(rx_idx);
  Serial.println(" bytes raw.");

  // ---- TX Echo Detection & Stripping ----
  // หาก RS485 transceiver ไม่ได้ปิด RE ระหว่าง TX จะได้ echo ของ frame ที่เราส่งออกไปคืนมา
  // ตรวจสอบโดยเปรียบเทียบ rx_buf กับ rtu_buf ที่เราส่งออกไปจริงๆ
  if (rx_idx >= tx_frame_size &&
      memcmp(rx_buf, rtu_buf, tx_frame_size) == 0) {
    Serial.print("[RS485] TX echo detected (");
    Serial.print(tx_frame_size);
    Serial.print(" bytes) — stripping. Remaining: ");
    int remaining = rx_idx - tx_frame_size;
    Serial.print(remaining);
    Serial.println(" bytes.");
    if (remaining > 0) {
      memmove(rx_buf, rx_buf + tx_frame_size, remaining);
    }
    rx_idx = remaining;
  }

  Serial.print("[RS485] Effective response: ");
  Serial.print(rx_idx);
  Serial.println(" bytes.");

  if (rx_idx == 0) {
    Serial.println("[ERR] No response — not forwarding to TCP client.");
    return;
  }

  printHex("RTU RX", rx_buf, rx_idx);

  // ตรวจสอบ CRC ที่ได้รับ
  if (rx_idx >= 3) {
    if (verifyCRC(rx_buf, rx_idx)) {
      Serial.println("[CRC] PASS ✓");
    } else {
      uint16_t calcCrc = calculateCRC(rx_buf, rx_idx - 2);
      uint16_t recvCrc = (uint16_t)rx_buf[rx_idx - 2] | ((uint16_t)rx_buf[rx_idx - 1] << 8);
      Serial.print("[CRC] FAIL ✗  Calculated=0x");
      Serial.print(calcCrc, HEX);
      Serial.print("  Received=0x");
      Serial.println(recvCrc, HEX);
      // Bug fix #4: ทิ้ง frame ที่ CRC ผิด — ห้ามส่ง corrupted data ให้ Master
      // Master จะ timeout และ retry เองโดยอัตโนมัติ
      Serial.println("[ERR] CRC failed — discarding corrupted frame.");
      return;
    }
  } else {
    Serial.println("[WARN] Response too short to verify CRC — discarding.");
    return;
  }

  // ---- ขั้นที่ 5: แปลง RTU → TCP แล้วส่งกลับ ----
  if (rx_idx < 3) {
    Serial.println("[ERR] RTU response too short to wrap in TCP — discarding.");
    return;
  }

  int pdu_len = rx_idx - 2;          // ตัด CRC 2 ไบต์ออก

  // Bug fix #1: ป้องกัน buffer overflow ใน tcp_resp
  // TCP_BUF_SIZE=256, MBAP header=6 → payload สูงสุด 250 bytes
  // Modbus max response (FC03 read 125 regs): 1+1+1+250 = 253 bytes PDU → เกินได้
  if (pdu_len > (TCP_BUF_SIZE - MBAP_HEADER_LEN)) {
    Serial.print("[ERR] pdu_len too large for TCP buffer: ");
    Serial.println(pdu_len);
    return;
  }

  uint8_t tcp_resp[TCP_BUF_SIZE];

  tcp_resp[0] = (uint8_t)(transId >> 8);   // Transaction ID High
  tcp_resp[1] = (uint8_t)(transId & 0xFF); // Transaction ID Low
  tcp_resp[2] = 0x00;                      // Protocol ID High
  tcp_resp[3] = 0x00;                      // Protocol ID Low
  tcp_resp[4] = (uint8_t)(pdu_len >> 8);   // Length High
  tcp_resp[5] = (uint8_t)(pdu_len & 0xFF); // Length Low
  memcpy(tcp_resp + MBAP_HEADER_LEN, rx_buf, pdu_len);

  int tcp_resp_total = MBAP_HEADER_LEN + pdu_len;
  printHex("TCP TX", tcp_resp, tcp_resp_total);

  client.write(tcp_resp, tcp_resp_total);
  Serial.print("[TCP] Response forwarded to client (");
  Serial.print(tcp_resp_total);
  Serial.println(" bytes).");
  Serial.println("----------------------------------------");
}