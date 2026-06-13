# lcd_display.py
import sqlite3
import time
from datetime import datetime

try:
    from luma.lcd import lcd12864
    from luma.core.interface.serial import spi
    from luma.core.render import canvas
    from PIL import ImageFont, ImageDraw
    LUMA_AVAILABLE = True
except ImportError:
    LUMA_AVAILABLE = False
    print("luma.lcd not available, running in simulation mode")

class LCDDisplay:
    def __init__(self):
        self.device = None
        self.font = None
        if LUMA_AVAILABLE:
            try:
                # SPI настройка
                serial = spi(port=0, device=0, gpio_DC=24, gpio_RST=25)
                self.device = lcd12864(serial)
                
                # Загрузка шрифта
                try:
                    self.font = ImageFont.truetype("/usr/share/fonts/TTF/DejaVuSans.ttf", 10)
                except:
                    self.font = ImageFont.load_default()
                    
                print("LCD initialized successfully")
            except Exception as e:
                print(f"LCD initialization failed: {e}")
                self.device = None
        else:
            print("Running in simulation mode")
            
    def get_sensor_data(self):
        """Получение последних данных из БД"""
        conn = sqlite3.connect("weather.db")
        c = conn.cursor()
        
        # Данные от Elbear
        c.execute('''SELECT temperature, humidity, pressure 
                     FROM sensor_data 
                     WHERE device = 'Elbear' 
                     ORDER BY timestamp DESC LIMIT 1''')
        elbear = c.fetchone()
        
        # Данные от Arduino
        c.execute('''SELECT co2, distance 
                     FROM sensor_data 
                     WHERE device = 'ArduinoMega' 
                     ORDER BY timestamp DESC LIMIT 1''')
        arduino = c.fetchone()
        
        conn.close()
        
        return {
            'temp': elbear[0] if elbear else None,
            'hum': elbear[1] if elbear else None,
            'press': elbear[2] if elbear else None,
            'co2': arduino[0] if arduino else None,
            'dist': arduino[1] if arduino else None
        }
    
    def format_text(self, data):
        """Форматирование текста для LCD"""
        lines = []
        
        if data['temp'] is not None:
            lines.append(f"T:{data['temp']:.1f}C  H:{data['hum']:.0f}%")
        else:
            lines.append("Waiting for data...")
            
        if data['press'] is not None:
            lines.append(f"P:{data['press']:.0f}mmHg")
            
        if data['co2'] is not None:
            lines.append(f"CO2:{data['co2']:.0f}ppm")
            
        if data['dist'] is not None:
            lines.append(f"Dist:{data['dist']}mm")
            
        lines.append(datetime.now().strftime("%H:%M:%S"))
        
        return "\n".join(lines)
    
    def update(self):
        """Обновление дисплея"""
        if self.device is None:
            # Режим симуляции — просто печатаем в консоль
            while True:
                data = self.get_sensor_data()
                text = self.format_text(data)
                print(f"LCD Display:\n{text}\n{'-'*20}")
                time.sleep(3)
        else:
            # Реальный дисплей
            while True:
                try:
                    data = self.get_sensor_data()
                    text = self.format_text(data)
                    
                    with canvas(self.device) as draw:
                        draw.rectangle(self.device.bounding_box, outline="black", fill="black")
                        draw.text((2, 2), text, fill="white", font=self.font)
                    
                    time.sleep(3)
                except Exception as e:
                    print(f"LCD update error: {e}")
                    time.sleep(5)

if __name__ == "__main__":
    display = LCDDisplay()
    display.update()
