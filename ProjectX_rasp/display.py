# lcd_display.py
import smbus2
import time

class LCDDisplay:
    def __init__(self, i2c_bus=1, i2c_addr=0x3F):
        self.bus = smbus2.SMBus(i2c_bus)
        self.addr = i2c_addr
        self.init_display()
    
    def init_display(self):
        # Инициализация дисплея
        self.send_command(0x33)
        self.send_command(0x32)
        self.send_command(0x28)
        self.send_command(0x0C)
        self.send_command(0x01)
        time.sleep(0.002)
    
    def send_command(self, cmd):
        self.bus.write_byte(self.addr, cmd)
    
    def display_weather(self, data):
        # Отображение данных на дисплее
        line1 = f"T:{data.get('temp', 0):.1f}C H:{data.get('hum', 0):.1f}%"
        line2 = f"P:{data.get('press', 0):.0f}mm"
        self.write_line(0, line1)
        self.write_line(1, line2)
    
    def write_line(self, line, text):
        addr = 0x80 if line == 0 else 0xC0
        self.send_command(addr)
        for char in text:
            self.bus.write_byte(self.addr, ord(char), 1)  # 1 = data

# Использование
lcd = LCDDisplay()
lcd.display_weather({'temp': 26.5, 'hum': 70.0, 'press': 760})
