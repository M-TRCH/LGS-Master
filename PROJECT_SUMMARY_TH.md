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
- ชื่อโหมดเดิมเคยชวนสับสน แต่ปัจจุบันปรับให้สื่อความหมายเป็น `local_mode_active` ซึ่งตรงกับพฤติกรรมจริงมากกว่า
- ใช้ credential MQTT แบบ plain text ใน source code

## 9. ข้อสังเกตเชิงนำไปใช้งานจริง

1. ค่าคอนฟิกหลักถูกรวมศูนย์ไว้ที่ `include/config.h` แล้ว
   - IP address
   - module type
   - MQTT broker
   - username/password
   - timeout/reconnect policy
   - pin mapping, protocol width และค่าพารามิเตอร์ของ LGS
   - หากต้องการเปลี่ยนพฤติกรรมระบบในภาพรวม ควรเริ่มแก้จากไฟล์นี้เป็นจุดแรก

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
   - ค่า `local_mode_active = true` หมายถึงเครื่องจะตอบสนองปุ่มหน้าเครื่องและไม่เปิด network path
   - ถ้าไม่กดปุ่มใดปุ่มหนึ่งค้างระหว่าง startup ระบบจะสลับไป network mode โดยตั้งค่าเป็น `false`

## 10. บทวิเคราะห์ปัญหา reconnect หลัง idle

อาการที่ตรวจพบ:

- client เชื่อมต่อครั้งแรก ส่งคำสั่งเสร็จ แล้วตัดการเชื่อมต่อเอง
- ถ้า reconnect ทันทีหรือในช่วงเวลาสั้น ๆ จะเชื่อมต่อได้ตามปกติ
- แต่ถ้าปล่อยทิ้งช่วงประมาณ 5 นาที attempt แรกหลังกลับมาอาจยังไม่เชื่อมต่อ
- ที่ panel ของ module ID `99` จะแสดงสีเหลือง `waiting` ใน attempt ที่ยังไม่สำเร็จ
- เมื่อ client ลองเชื่อมต่ออีกครั้งติด ๆ กัน สถานะจะเปลี่ยนเป็นสีเขียว `connected`

หลักฐานจากโค้ด:

- ไฟสถานะ TCP ถูกตัดสินจากผลลัพธ์ของ `tcp_server_update()` ใน `src/main.cpp`
- ถ้า `tcp_server_update()` คืนค่า `false` ระบบจะขึ้น `waiting`
- ถ้าฟังก์ชันเดียวกันคืนค่า `true` ระบบจะขึ้น `connected`
- ดังนั้นกรณีที่ไฟยังเหลือง แปลว่าฝั่ง firmware ยังไม่ได้ถือว่ามี active client ในรอบนั้น

ข้อสรุปเชิงวิเคราะห์:

1. ปัญหานี้ไม่ได้ชี้ไปที่การ parse packet หรือ `transition` duplicate เป็น root cause หลัก
   - ถ้า TCP connect สำเร็จแล้วแต่ packet ถูก reject สีสถานะควรยังเป็นเขียว
   - แต่อาการจริงคือใน attempt ที่มีปัญหาไฟยังคงเป็นเหลือง

2. ฝั่ง firmware มี defect ที่ควรแก้อยู่จริงในลอจิกจัดการ client
   - ใน `src/ethernet_utils.cpp` โค้ดเรียก `tcp_server.accept()` ก่อน
   - จากนั้นจึงค่อยตรวจว่า client เก่าหลุดหรือ timeout แล้วหรือยัง
   - ลำดับนี้เปิดโอกาสให้ attempt แรกของการ reconnect ถูกพลาดได้หนึ่งรอบ ถ้า slot เดิมยังไม่ถูกเคลียร์

3. อย่างไรก็ตาม threshold ที่สัมพันธ์กับเวลา `ประมาณ 5 นาที` ไม่ได้ถูกสร้างจาก application logic ของ firmware โดยตรง
   - ค่า timeout ฝั่ง firmware สำหรับ client ปัจจุบันคือ `120000 ms` หรือ `2 นาที`
   - ในโค้ดไม่มี timer `5 นาที` สำหรับ TCP reconnect
   - จึงมีน้ำหนักว่าปัญหาอาจเกิดจาก interaction ระหว่าง client behavior กับ network/socket state ระดับต่ำกว่า application

4. พฤติกรรมของ client ที่ `connect -> send command -> disconnect ทันที` เป็นตัวกระตุ้นปัญหาได้
   - เพราะฝั่ง firmware รับรู้การหลุดของ client ด้วยการ polling ใน loop
   - ถ้า client ปิดการเชื่อมต่อเร็วมาก สถานะเดิมอาจค้างอยู่ชั่วคราวก่อนจะถูกเคลียร์
   - พฤติกรรมนี้อธิบายได้ว่าทำไม attempt ถัดไปแบบติด ๆ กันจึงมีโอกาสสำเร็จหลัง state เดิมถูกเก็บกวาดแล้ว

ข้อสรุปสุดท้ายสำหรับ incident นี้:

- firmware มีส่วนเสี่ยงจากลอจิก stale client state และควรแก้
- แต่จากหลักฐานที่มี ยังไม่พอจะยืนยันว่า firmware application เป็นตัวสร้างเงื่อนไข `5 นาที` ด้วยตัวเอง
- root cause ที่เป็นไปได้มากที่สุดคือการซ้อนกันของ
  - defect ในลอจิก accept/cleanup ฝั่ง firmware
  - รูปแบบการ disconnect ของ client
  - และ network/socket state หลัง idle ระยะหนึ่ง

## 11. แนวทางแก้ปัญหาที่เสนอ

### ระยะสั้น

1. ปรับพฤติกรรม client
   - หลังรับ response แล้วให้ค้าง connection ต่ออีกช่วงสั้น ๆ เช่น `1-3 วินาที` ก่อน disconnect
   - หรือเปลี่ยนเป็น reuse connection เดิมแทนการ connect/disconnect ทุกคำสั่ง

2. เก็บ log เพื่อแยกสาเหตุให้ชัด
   - ดู Serial log ว่า attempt ที่ล้มเหลวมีข้อความ `Client connected:` หรือไม่
   - ถ้าไม่มี แปลว่า connection ยังไม่ถึงชั้น application ของ firmware
   - ถ้ามี แต่ยังทำงานต่อไม่ได้ ให้ไล่ต่อที่ packet handling และ client behavior

### ระยะกลาง

1. แก้ลอจิก `tcp_server_update()`
   - ตรวจและเคลียร์ client เก่าก่อน `accept()` client ใหม่
   - ลดโอกาสพลาด reconnect attempt แรกจาก stale slot

2. เพิ่ม logging สำหรับ state transition ของ TCP
   - log ตอนพบว่า client เก่าหลุด
   - log ตอน timeout
   - log ตอนรับ client ใหม่
   - log เหตุผลที่ไม่รับ client ใหม่ในรอบนั้น

3. พิจารณา reset state ที่ค้างหลัง disconnect
   - เช่นข้อมูล client metadata และตัวแปรที่เกี่ยวข้องกับ session ปัจจุบัน

### ระยะยาว

1. ออกแบบ reconnect policy ให้ชัดทั้งสองฝั่ง
   - ฝั่ง client ควรกำหนด retry interval, connect timeout และ post-response delay
   - ฝั่ง firmware ควรกำหนด session cleanup policy ให้ชัดเจนและสม่ำเสมอ

2. เพิ่ม test scenario สำหรับ TCP lifecycle
   - connect -> command -> disconnect
   - reconnect ทันที
   - reconnect หลัง idle 2 นาที
   - reconnect หลัง idle 5 นาที
   - reconnect ซ้ำหลายครั้งติดกัน

3. ถ้ายังพบปัญหาเดิมหลังแก้ลอจิก firmware แล้ว
   - ควรตรวจต่อที่ ARP aging, TCP half-close behavior, และ library behavior ของ Ethernet stack บน Opta

## 12. สรุปสั้น

โปรเจคนี้เหมาะกับงานควบคุม LGS ผ่าน TCP บนฮาร์ดแวร์เฉพาะ โดยมีจุดแข็งที่ flow การทำงานตรงไปตรงมา, แยกโมดูลชัด, และมี watchdog กับ logging รองรับ แต่ถ้าจะนำไปใช้งานจริงในระบบที่ต้องดูแลง่ายและขยายต่อได้ ควรเร่งจัดการเรื่อง configuration management, security ของ MQTT, test coverage และความชัดเจนของ operational mode เป็นลำดับแรก

สำหรับ incident เรื่อง reconnect หลัง idle ข้อสรุปปัจจุบันคือ firmware มีจุดที่ควรแก้ในลอจิกการจัดการ TCP client อย่างชัดเจน แต่สาเหตุเชิงเวลา `5 นาที` ยังชี้ว่าต้องพิจารณาพฤติกรรมฝั่ง client และ network stack ร่วมด้วย ไม่ควรสรุปว่าเป็นความผิดของ application firmware เพียงด้านเดียว