import serial
import time
from datetime import datetime

# ================= การตั้งค่า =================
COM_PORT = 'COM23'  # เปลี่ยนเป็น COM Port ของสาย USB-RS485 ของคุณ
BAUD_RATE = 9600   # ต้องตรงกับที่ตั้งไว้ใน Opta
SLAVE_ID  = 62     # ID ของ slave ที่จะจำลอง
# ============================================

FUNC_WRITE_COIL       = 0x05
FUNC_READ_COILS       = 0x01
FUNC_READ_HOLDING_REG = 0x03

def crc16(data: bytes) -> int:
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc

def append_crc(data: bytes) -> bytes:
    crc = crc16(data)
    return data + bytes([crc & 0xFF, (crc >> 8) & 0xFF])

def check_crc(data: bytes) -> bool:
    if len(data) < 3:
        return False
    payload = data[:-2]
    recv_crc = data[-2] | (data[-1] << 8)
    return crc16(payload) == recv_crc

def build_response(request: bytes) -> bytes | None:
    if len(request) < 4:
        return None

    slave_id = request[0]
    func     = request[1]

    if slave_id != SLAVE_ID:
        return None  # ไม่ใช่ slave ของเรา

    if func == FUNC_WRITE_COIL and len(request) == 8:
        # FC05: echo กลับ frame เดิม
        return bytes(request)

    if func == FUNC_READ_COILS and len(request) == 8:
        # FC01: ตอบกลับด้วย 1 byte coil data (ค่า 0x00 ทั้งหมด)
        qty    = (request[4] << 8) | request[5]
        n_bytes = (qty + 7) // 8
        response = bytes([slave_id, func, n_bytes]) + bytes(n_bytes)
        return append_crc(response)

    if func == FUNC_READ_HOLDING_REG and len(request) == 8:
        # FC03: ตอบกลับด้วยค่า 0x0000 ทุก register
        qty      = (request[4] << 8) | request[5]
        n_bytes  = qty * 2
        response = bytes([slave_id, func, n_bytes]) + bytes(n_bytes)
        return append_crc(response)

    return None

def start_sniffer():
    try:
        # 🔥 ไฮไลท์การแก้: เพิ่ม inter_byte_timeout=0.005 เข้าไปตอนเปิดพอร์ต
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=0.1, inter_byte_timeout=0.005)
        print(f"🟢 Slave Simulator started on {COM_PORT} at {BAUD_RATE} bps (Slave ID={SLAVE_ID})")
        print("Waiting for Modbus RTU requests... (Press Ctrl+C to stop)\n")

        while True:
            # ให้อ่านมาทีเดียวเยอะๆ เลย ถ้าสายว่างเกิน 5ms คำสั่ง read() จะคืนค่าเฟรมที่สมบูรณ์ออกมาให้เองทันที
            buffer = ser.read(1024)

            if len(buffer) >= 4:
                timestamp = datetime.now().strftime('%H:%M:%S.%f')[:-3]
                hex_rx = ' '.join([f'{b:02X}' for b in buffer])
                crc_ok = check_crc(buffer)
                print(f"[{timestamp}] 📦 RX: {hex_rx}  CRC={'OK' if crc_ok else 'FAIL'}")

                if crc_ok:
                    response = build_response(bytes(buffer))
                    if response:
                        # 🔥 ไฮไลท์การแก้: ต้องรอให้ Opta วางสายให้สนิทก่อน (Opta หน่วง 3ms เราต้องรอให้มากกว่า)
                        time.sleep(0.02)  # หน่วง 20ms เพื่อความชัวร์ระดับอุตสาหกรรม
                        
                        ser.write(response)
                        ser.flush() # บังคับให้ดันข้อมูลออกจากบัฟเฟอร์ USB ให้หมด
                        
                        hex_tx = ' '.join([f'{b:02X}' for b in response])
                        timestamp = datetime.now().strftime('%H:%M:%S.%f')[:-3]
                        print(f"[{timestamp}] 📤 TX: {hex_tx}\n") # ใส่ \n ให้เว้นบรรทัดดูง่ายขึ้น

    except serial.SerialException as e:
        print(f"🔴 Error opening port {COM_PORT}: {e}")
    except KeyboardInterrupt:
        print("\n🛑 Stopped.")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == '__main__':
    start_sniffer()