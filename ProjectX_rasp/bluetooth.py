# bt_receiver.py
import socket
import time
import threading
import sqlite3
import json
import logging
from datetime import datetime

# Настройка логирования
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# MAC-адреса устройств (замените на свои)
DEVICES = {
    "Elbear": "XX:XX:XX:XX:XX:XX",      # MAC адрес HC-05 на Elbear
    "ArduinoMega": "YY:YY:YY:YY:YY:YY"  # MAC адрес HC-05 на Arduino Mega
}

class BluetoothReceiver:
    def __init__(self, db_name="weather.db"):
        self.db_name = db_name
        self.init_database()
        
    def init_database(self):
        """Инициализация базы данных SQLite"""
        conn = sqlite3.connect(self.db_name)
        c = conn.cursor()
        c.execute('''CREATE TABLE IF NOT EXISTS sensor_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT,
            device TEXT,
            raw_data TEXT,
            temperature REAL,
            humidity REAL,
            pressure REAL,
            light INTEGER,
            sound INTEGER,
            flame INTEGER,
            leak INTEGER,
            co2 REAL,
            distance INTEGER,
            ax INTEGER,
            ay INTEGER,
            az INTEGER
        )''')
        conn.commit()
        conn.close()
        
    def parse_data(self, device, raw_data):
        """Парсинг данных от разных устройств"""
        parsed = {
            'device': device,
            'raw_data': raw_data,
            'temperature': None,
            'humidity': None,
            'pressure': None,
            'light': None,
            'sound': None,
            'flame': None,
            'leak': None,
            'co2': None,
            'distance': None,
            'ax': None,
            'ay': None,
            'az': None
        }
        
        # Парсинг данных от Elbear
        if device == "Elbear":
            parts = raw_data.split(',')
            for part in parts:
                if part.startswith('T:'):
                    parsed['temperature'] = float(part[2:])
                elif part.startswith('H:'):
                    parsed['humidity'] = float(part[2:])
                elif part.startswith('P:'):
                    parsed['pressure'] = float(part[2:])
                elif part.startswith('L:'):
                    parsed['light'] = int(part[2:])
                elif part.startswith('SND:'):
                    parsed['sound'] = int(part[4:])
                elif part.startswith('FLAME:'):
                    parsed['flame'] = int(part[6:])
                elif part.startswith('LEAK:'):
                    parsed['leak'] = int(part[5:])
                    
        # Парсинг данных от Arduino Mega
        elif device == "ArduinoMega":
            parts = raw_data.split(',')
            for part in parts:
                if part.startswith('AX:'):
                    parsed['ax'] = int(part[3:])
                elif part.startswith('AY:'):
                    parsed['ay'] = int(part[3:])
                elif part.startswith('AZ:'):
                    parsed['az'] = int(part[3:])
                elif part.startswith('CO2:'):
                    parsed['co2'] = float(part[4:])
                elif part.startswith('DIST:'):
                    parsed['distance'] = int(part[5:])
                    
        return parsed
    
    def save_to_database(self, data):
        """Сохранение распарсенных данных в БД"""
        conn = sqlite3.connect(self.db_name)
        c = conn.cursor()
        c.execute('''INSERT INTO sensor_data 
            (timestamp, device, raw_data, temperature, humidity, pressure, 
             light, sound, flame, leak, co2, distance, ax, ay, az)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)''',
            (datetime.now().isoformat(),
             data['device'],
             data['raw_data'],
             data['temperature'],
             data['humidity'],
             data['pressure'],
             data['light'],
             data['sound'],
             data['flame'],
             data['leak'],
             data['co2'],
             data['distance'],
             data['ax'],
             data['ay'],
             data['az']))
        conn.commit()
        conn.close()
        
    def connect_device(self, mac_address, device_name, port=1):
        """Подключение к Bluetooth устройству через сокет"""
        while True:
            sock = None
            try:
                # Создание RFCOMM сокета
                sock = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
                
                # Подключение
                logger.info(f"Connecting to {device_name} ({mac_address})...")
                sock.connect((mac_address, port))
                logger.info(f"Connected to {device_name}")
                
                # Настройка таймаута
                sock.settimeout(1.0)
                
                # Буфер для данных
                buffer = ""
                
                while True:
                    try:
                        # Чтение данных
                        data = sock.recv(1024).decode('utf-8', errors='ignore')
                        if data:
                            buffer += data
                            # Разделение по строкам
                            if '\n' in buffer:
                                lines = buffer.split('\n')
                                for line in lines[:-1]:
                                    line = line.strip()
                                    if line:
                                        logger.info(f"[{device_name}] {line}")
                                        parsed = self.parse_data(device_name, line)
                                        self.save_to_database(parsed)
                                buffer = lines[-1]
                    except socket.timeout:
                        continue
                    except Exception as e:
                        logger.error(f"Read error from {device_name}: {e}")
                        break
                        
            except Exception as e:
                logger.error(f"Connection failed for {device_name}: {e}")
                time.sleep(5)  # Пауза перед переподключением
            finally:
                if sock:
                    sock.close()
                    logger.info(f"Disconnected from {device_name}")
                    
    def start(self):
        """Запуск всех потоков приёма"""
        threads = []
        for device_name, mac_address in DEVICES.items():
            thread = threading.Thread(
                target=self.connect_device,
                args=(mac_address, device_name),
                daemon=True
            )
            thread.start()
            threads.append(thread)
            time.sleep(2)  # Пауза между подключениями
            
        # Ожидание завершения (бесконечно)
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            logger.info("Stopping receiver...")

if __name__ == "__main__":
    receiver = BluetoothReceiver()
    receiver.start()
