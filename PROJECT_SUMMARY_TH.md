# สรุปโปรเจค LGS-Master

## 1. โปรเจคนี้ทำอะไร

LGS-Master เป็นเฟิร์มแวร์สำหรับบอร์ด Arduino Opta ที่ทำหน้าที่เป็นตัวกลางระหว่างระบบภายนอกกับโมดูล LGS ผ่าน RS485 โดยรับคำสั่งผ่าน TCP, แปลงคำสั่งไปควบคุม LGS, ตอบกลับผลลัพธ์ให้ client และส่ง log/สถานะออกทาง Serial และ MQTT

สแตกหลักของโปรเจค:

- Framework: Arduino บน PlatformIO
- Board: Opta
- Platform: `ststm32`
- การสื่อสารกับ LGS: RS485
- การสื่อสารกับระบบภายนอก: TCP Server
- การส่ง log/telemetry: Serial + MQTT

## 2. ลำดับการทำงานของระบบ

ลำดับหลักใน `setup()` และ `loop()` มีดังนี้

1. ตั้งค่า device configuration
   - กำหนด `ModuleType::STANDARD`
   - กำหนด IP แบบ hardcoded เป็น `192.168.0.99`
   - กำหนด firmware version เป็น `11/09/2025`

2. เริ่มระบบ log
   - เปิด log level เป็น `LOG_VERBOSE`
   - log ออกทาง Serial

3. เริ่มระบบพื้นฐาน
   - ตั้งค่า pin, relay, switch, LED
   - เปิด Serial
   - รอ startup delay ของโมดูล
   - ตรวจปุ่มเพื่อเลือกโหมดเริ่มต้น

4. เริ่ม LGS controller
   - เปิดบัส RS485
   - ถ้าอยู่ในโหมด developer จะไล่สีบน panel เพื่อทดสอบ

5. ถ้าไม่อยู่ใน developer mode
   - เริ่ม Ethernet ด้วย Static IP
   - ถ้าสาย LAN ไม่เสียบ จะขึ้นสถานะ error
   - ถ้าลิงก์พร้อม จะเปิด TCP server และเชื่อม MQTT

6. เริ่ม watchdog
   - timeout 30 วินาที
   - ใน `loop()` จะ kick เป็นระยะ

7. ใน `loop()`
   - ตรวจ event จากปุ่มต่าง ๆ
   - ถ้า network พร้อม จะอัปเดต TCP server และ MQTT
   - ถ้ามี TCP packet เข้ามา จะ
     - parse packet
     - ตรวจ checksum
     - กัน packet ซ้ำด้วย `transition`
     - ส่ง response รอบแรก
     - execute คำสั่งกับ LGS
     - ส่ง response รอบสอง

## 3. โปรโตคอล TCP ที่ระบบรองรับ

ระบบฟัง TCP ที่ port `2000`

รูปแบบ packet ตัวอย่าง:

```text
B01R08C01Q0002L01M01T00N01D0000S15
```

ความหมาย field:

- `B` = จุดเริ่ม packet
- `R` = row
- `C` = column
- `Q` = quantity
- `L` = color
- `M` = command
- `T` = return status
- `N` = transition number
- `D` = device
- `S` = checksum

คำสั่งสำคัญ:

- `CMD_OFF = 0`
- `CMD_ON = 1`
- `CMD_REQUEST = 2`
- `CMD_REBOOT = 9`

สถานะตอบกลับที่ใช้งานในโค้ด:

- `1` = ผ่านรอบแรก (`FIRST_SUCCEED`)
- `2` = ผ่านหลัง execute (`SECOND_SUCCEED`)
- `3` = ไม่มี action (`NO_ACTION`)
- `4` = idle
- `5` = busy
- `9` = fail

ข้อจำกัดเชิง implementation:

- รองรับ TCP client พร้อมกันเพียง 1 client
- ตรวจ packet ซ้ำโดยอิง `device` + `transition`
- ถ้า checksum ไม่ตรงหรือ device เกินขอบเขต จะทิ้ง packet

## 4. การแมปคำสั่งไปยัง LGS

### Standard module

- รองรับ row `1..8`, column `1..8`
- รองรับสี `1..4`
  - `1 = red`
  - `2 = green`
  - `3 = blue`
  - `4 = yellow`
- มีการกลับ row จาก `1..8` ไปเป็น `8..1` ก่อนส่งให้ LGS

### Narcotic module

- โค้ดรองรับไว้บางส่วน แต่ `main.cpp` ตั้งค่าเริ่มต้นเป็น `ModuleType::STANDARD`
- รองรับ row `0..9`, column `1..8`
- รองรับเฉพาะสีแดง
- คำสั่ง OFF จะพยายามอ่าน sensor ก่อน ถ้าโมดูลยังไม่อยู่ตำแหน่งจะไม่ปิด

## 5. คำสั่งที่เกี่ยวข้องกับโปรเจค

### คำสั่งที่ยืนยันได้จากรีโป

VS Code task ที่มีใน workspace:

```powershell
C:\Users\mteer\.platformio\penv\Scripts\platformio.exe run
```

คำสั่ง build ตรงจาก root ของโปรเจค:

```powershell
C:\Users\mteer\.platformio\penv\Scripts\platformio.exe run
```

### คำสั่ง PlatformIO ที่เกี่ยวข้องโดยตรง

อัปโหลดเฟิร์มแวร์:

```powershell
C:\Users\mteer\.platformio\penv\Scripts\platformio.exe run -t upload
```

เปิด serial monitor ที่ baud `9600`:

```powershell
C:\Users\mteer\.platformio\penv\Scripts\platformio.exe device monitor -b 9600
```

ล้าง build output:

```powershell
C:\Users\mteer\.platformio\penv\Scripts\platformio.exe run -t clean
```

หมายเหตุ:

- `platformio.ini` ระบุ `monitor_speed = 9600`
- คำสั่ง upload ต้องมีพอร์ตและสิทธิ์เข้าถึงอุปกรณ์พร้อมใช้งาน

## 6. ไฟล์สำคัญของระบบ

- `src/main.cpp` จุดเริ่มต้นของการทำงานทั้งหมด
- `src/system.cpp` ตั้งค่า pin, relay, startup, watchdog, ปุ่มกด
- `src/ethernet_utils.cpp` Ethernet, TCP server, MQTT, packet parsing
- `src/tcp_lgs_bridge.cpp` แปลคำสั่ง TCP ไปเป็นคำสั่ง LGS
- `src/lgs_controller.cpp` ควบคุม LGS bus, LED, status, soft reset
- `src/logger.cpp` ระบบ log
- `include/*.h` ค่าคงที่, enum, และ public interface

## 7. จุดเด่นของโปรเจค

- โครงสร้างแยกหน้าที่ค่อนข้างชัด ระหว่าง system, network, bridge, controller และ logger
- มี watchdog ลดโอกาสค้างแล้วไม่ฟื้นตัว
- มี packet validation พื้นฐาน ทั้ง checksum และ duplicate transition
- มี MQTT สำหรับส่ง log/สถานะออกไปยังระบบภายนอก
- มี developer mode สำหรับทดสอบ panel และโมดูลจากปุ่มหน้าเครื่อง
- มีการตอบกลับ TCP สองช่วง ช่วยให้ระบบภายนอกเห็นสถานะก่อนและหลัง execute

## 8. จุดด้อยของโปรเจค

- ค่าคอนฟิกสำคัญเป็น hardcoded เกือบทั้งหมด เช่น module type, device IP, MQTT broker, username/password
- รองรับ TCP client ได้เพียง 1 รายพร้อมกัน
- MQTT reconnect interval ยาวถึง 15 นาที ซึ่งอาจช้าเกินไปสำหรับงาน production
- ไม่มี test automation ในโฟลเดอร์ `test/`
- เอกสารบางส่วนยังไม่ครบหรือไม่ตรงกับโค้ด เช่น `LOG_MQTT_MIGRATION.md` ยังว่าง และตัวอย่างใน `LOGGING_GUIDE.md` มี enum ที่ไม่ตรงจริงบางจุด
- developer mode / normal mode ตั้งชื่อแปรชวนสับสน: `dev_mode_activated` เริ่มต้นเป็น `true` แต่ถ้ากดปุ่มครบชุดช่วงบูตจะถูกตั้งเป็น `false` และไปเปิด network mode แทน
- ใช้ credential MQTT แบบ plain text ใน source code

## 9. ข้อสังเกตเชิงนำไปใช้งานจริง

1. ก่อนใช้จริงควรย้ายค่าคอนฟิกสำคัญออกจาก source code
   - IP address
   - module type
   - MQTT broker
   - username/password
   - timeout/reconnect policy

2. ถ้าจะใช้งานในระบบที่ต้องการความต่อเนื่องสูง ควรทบทวน logic MQTT reconnect
   - การรอ 15 นาทีต่อรอบอาจทำให้ monitoring ขาดช่วงนานเกินไป

3. ถ้าจะใช้งานหลาย client พร้อมกัน โค้ดปัจจุบันยังไม่รองรับ
   - ตอนนี้รับได้ทีละ 1 TCP client

4. ควรนิยาม packet protocol ภายนอกให้เป็นเอกสารกลางอย่างเป็นทางการ
   - ตอนนี้ข้อมูลโปรโตคอลกระจายอยู่ใน comment และ implementation

5. ควรเพิ่ม test อย่างน้อยในระดับ protocol parsing และ command mapping
   - ตรวจ checksum
   - ตรวจ duplicate packet
   - ตรวจ row/column/color range
   - ตรวจ mapping command/status

6. ถ้าจะ build บน environment ที่ case-sensitive ควรตรวจชื่อไฟล์ include ให้ตรงตัวพิมพ์
   - ในรีโปมีไฟล์ `LGSbus.h` แต่มี include เป็น `LGSBus.h`

7. ระวังพฤติกรรม startup mode
   - logic ปัจจุบันทำให้คนอ่านชื่อแปรอาจเข้าใจผิดว่า `dev_mode_activated = true` หมายถึงเปิด developer mode แบบตั้งใจ แต่จริง ๆ network mode จะเริ่มเมื่อค่าดังกล่าวเป็น `false`

## 10. สรุปสั้น

โปรเจคนี้เหมาะกับงานควบคุม LGS ผ่าน TCP บนฮาร์ดแวร์เฉพาะ โดยมีจุดแข็งที่ flow การทำงานตรงไปตรงมา, แยกโมดูลชัด, และมี watchdog กับ logging รองรับ แต่ถ้าจะนำไปใช้งานจริงในระบบที่ต้องดูแลง่ายและขยายต่อได้ ควรเร่งจัดการเรื่อง configuration management, security ของ MQTT, test coverage และความชัดเจนของ operational mode เป็นลำดับแรก