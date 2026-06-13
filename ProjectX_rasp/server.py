#!/usr/bin/env python3
"""
Weather Station Server for Raspberry Pi
Работает через нативные Bluetooth-сокеты Linux (без pybluez)
"""

import socket
import struct
import threading
import json
import time
import os
from datetime import datetime
from flask import Flask, jsonify, render_template_string
from flask_socketio import SocketIO
import smbus2

# ==================== КОНФИГУРАЦИЯ ====================
MAC_ARDUINO = "00:11:22:33:44:55"  # HC-05 Arduino
MAC_ELBEAR  = "00:11:22:33:44:66"  # HC-05 Elbear

HTTP_PORT = 5000
TCP_PORT  = 5001
LOG_FILE  = '/tmp/weather_station.log'

# Bluetooth constants (из linux/bluetooth.h)
AF_BLUETOOTH = 31
BTPROTO_RFCOMM = 3
BDADDR_BRCM = 2  # адрес в формате "00:11:22:33:44:55"

# ==================== FLASK ====================
app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

weather_data = {
    'arduino': {
        'lm75a_temp': None, 'dht_temp': None, 'dht_humidity': None,
        'bmp_temp': None, 'bmp_pressure': None, 'timestamp': None
    },
    'elbear': {
        'thp_temp': None, 'thp_humidity': None, 'thp_pressure': None,
        'wind_speed': None, 'wind_direction': None, 'rainfall': None,
        'radiation': None, 'light_intensity': None, 'sound_level': None,
        'gas_level': None, 'co_level': None, 'distance': None,
        'timestamp': None
    },
    'last_update': None
}

# ==================== УТИЛИТЫ ====================
def log_message(message):
    try:
        timestamp = datetime.now().isoformat()
        with open(LOG_FILE, 'a') as f:
            f.write(f"[{timestamp}] {message}\n")
        print(f"[{timestamp}] {message}")
    except Exception as e:
        print(f"Log error: {e}")


def mac_to_bluetooth_addr(mac_str):
    """
    Преобразует MAC-адрес вида '00:11:22:33:44:55'
    в бинарный формат для sockaddr_rc (6 байт в обратном порядке).
    """
    parts = mac_str.split(':')
    if len(parts) != 6:
        raise ValueError(f"Invalid MAC: {mac_str}")
    # Bluetooth хранит адрес в little-endian порядке байт
    return bytes([int(p, 16) for p in reversed(parts)])


# ==================== BLUETOOTH КЛИЕНТ (нативный socket) ====================
class BluetoothClient:
    """Подключается к HC-05 через нативный RFCOMM-сокет Linux"""

    def __init__(self, mac, name):
        self.mac = mac
        self.name = name
        self.sock = None
        self.running = False
        self.buffer = ""

    def connect(self):
        max_retries = 10
        for attempt in range(1, max_retries + 1):
            try:
                log_message(f"[{self.name}] Attempt {attempt} to {self.mac}")
                self.sock = socket.socket(AF_BLUETOOTH, socket.SOCK_STREAM, BTPROTO_RFCOMM)

                # sockaddr_rc: family(2) + bdaddr(6) + channel(1) + padding(1)
                addr = struct.pack(
                    '<H6sB',
                    AF_BLUETOOTH,
                    mac_to_bluetooth_addr(self.mac),
                    1  # RFCOMM channel 1 (стандарт для HC-05 SPP)
                )
                self.sock.connect(addr)
                self.sock.settimeout(5.0)
                log_message(f"[{self.name}] Connected to {self.mac}")
                return True
            except Exception as e:
                log_message(f"[{self.name}] Connection failed: {e}")
                self.disconnect()
                time.sleep(3)
        return False

    def disconnect(self):
        if self.sock:
            try:
                self.sock.close()
            except:
                pass
            self.sock = None

    def read_loop(self, callback):
        self.running = True
        while self.running:
            try:
                if not self.sock:
                    if not self.connect():
                        time.sleep(5)
                        continue

                data = self.sock.recv(1024)
                if not data:
                    log_message(f"[{self.name}] Connection closed")
                    self.disconnect()
                    time.sleep(2)
                    continue

                text = data.decode('utf-8', errors='ignore')
                self.buffer += text

                while '\n' in self.buffer:
                    line, self.buffer = self.buffer.split('\n', 1)
                    line = line.strip()
                    if line:
                        callback(self.name, line)

            except OSError as e:
                log_message(f"[{self.name}] Socket error: {e}")
                self.disconnect()
                time.sleep(2)
            except Exception as e:
                log_message(f"[{self.name}] Read error: {e}")
                self.disconnect()
                time.sleep(2)

    def stop(self):
        self.running = False
        self.disconnect()


# ==================== ПАРСИНГ ====================
def parse_arduino_data(data_str):
    try:
        parts = data_str.split('|')
        if parts[0] == 'ARDUINO' and len(parts) >= 7:
            def sf(v):
                f = float(v)
                return None if f == -999.0 else f
            weather_data['arduino'] = {
                'lm75a_temp': sf(parts[1]), 'dht_temp': sf(parts[2]),
                'dht_humidity': sf(parts[3]), 'bmp_temp': sf(parts[4]),
                'bmp_pressure': sf(parts[5]),
                'timestamp': datetime.now().isoformat()
            }
            weather_data['last_update'] = datetime.now().isoformat()
            return True
    except Exception as e:
        log_message(f"Parse Arduino error: {e}")
    return False


def parse_elbear_data(data_str):
    try:
        parts = data_str.split('|')
        if parts[0] == 'ELBEAR' and len(parts) >= 14:
            def sf(v):
                f = float(v)
                return None if f == -999.0 else f
            weather_data['elbear'] = {
                'thp_temp': sf(parts[1]), 'thp_humidity': sf(parts[2]),
                'thp_pressure': sf(parts[3]), 'wind_speed': sf(parts[4]),
                'wind_direction': sf(parts[5]), 'rainfall': sf(parts[6]),
                'radiation': sf(parts[7]), 'light_intensity': sf(parts[8]),
                'sound_level': sf(parts[9]), 'gas_level': sf(parts[10]),
                'co_level': sf(parts[11]), 'distance': sf(parts[12]),
                'timestamp': datetime.now().isoformat()
            }
            weather_data['last_update'] = datetime.now().isoformat()
            return True
    except Exception as e:
        log_message(f"Parse Elbear error: {e}")
    return False


def handle_data(source, data):
    log_message(f"[{source}] {data}")
    updated = False
    if source == 'Arduino':
        updated = parse_arduino_data(data)
    elif source == 'Elbear':
        updated = parse_elbear_data(data)
    if updated:
        socketio.emit('data_update', weather_data)


# ==================== TCP СЕРВЕР ====================
def tcp_server():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('0.0.0.0', TCP_PORT))
    sock.listen(5)
    log_message(f"TCP server on port {TCP_PORT}")

    while True:
        try:
            conn, addr = sock.accept()
            def handle_client(c):
                try:
                    while True:
                        data = c.recv(1024).decode('utf-8')
                        if not data:
                            break
                        data = data.strip()
                        if data.startswith('ARDUINO'):
                            handle_data('Arduino', data)
                        elif data.startswith('ELBEAR'):
                            handle_data('Elbear', data)
                except Exception as e:
                    log_message(f"TCP client error: {e}")
                finally:
                    c.close()
            threading.Thread(target=handle_client, args=(conn,), daemon=True).start()
        except Exception as e:
            log_message(f"TCP server error: {e}")
            time.sleep(1)


# ==================== LCD ====================
class LCDDisplay:
    def __init__(self, i2c_bus=1, i2c_addr=0x3F):
        self.enabled = True
        try:
            self.bus = smbus2.SMBus(i2c_bus)
            self.addr = i2c_addr
            self._init()
        except Exception as e:
            log_message(f"LCD init error: {e}")
            self.enabled = False

    def _cmd(self, cmd):
        if self.bus:
            self.bus.write_byte(self.addr, cmd)

    def _init(self):
        self._cmd(0x33); self._cmd(0x32); self._cmd(0x28)
        self._cmd(0x0C); self._cmd(0x06); self._cmd(0x01)
        time.sleep(0.002)

    def _write_line(self, line_num, text):
        if not self.bus:
            return
        addr = 0x80 if line_num == 0 else 0xC0
        self._cmd(addr)
        for ch in text[:16]:
            self.bus.write_byte_data(self.addr, 0x40, ord(ch))

    def update(self, data):
        if not self.enabled:
            return
        a = data.get('arduino', {})
        t, h, p = a.get('dht_temp'), a.get('dht_humidity'), a.get('bmp_pressure')
        l1 = f"T:{t:.1f}C H:{h:.1f}%" if t is not None and h is not None else "No data"
        l2 = f"P:{p:.0f}mmHg" if p is not None else "No data"
        self._write_line(0, l1)
        self._write_line(1, l2)


def lcd_loop():
    lcd = LCDDisplay()
    while True:
        try:
            lcd.update(weather_data)
        except Exception as e:
            log_message(f"LCD error: {e}")
        time.sleep(5)


# ==================== WEB ====================
HTML_TEMPLATE = """
<!DOCTYPE html>
<html><head><title>Weather Station</title><meta charset="UTF-8">
<style>
body{font-family:Arial;margin:20px;background:#f0f0f0}
.container{max-width:1200px;margin:0 auto}
.box{background:#fff;padding:20px;margin:10px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,.1)}
h2{color:#666;border-bottom:2px solid #ddd;padding-bottom:10px}
.row{display:flex;justify-content:space-between;padding:8px 0;border-bottom:1px solid #eee}
.label{font-weight:bold}.value{color:#2196F3}
</style></head><body>
<div class="container">
<h1>🌤️ Weather Station</h1>
<div>Last update: <span id="ts">-</span></div>
<div class="box"><h2>📡 Arduino</h2><div id="arduino">Loading...</div></div>
<div class="box"><h2>🤖 Elbear</h2><div id="elbear">Loading...</div></div>
</div>
<script src="https://cdnjs.cloudflare.com/ajax/libs/socket.io/4.0.1/socket.io.js"></script>
<script>
const socket = io();
socket.on('data_update', d => render(d));
fetch('/api/data').then(r=>r.json()).then(render);
function render(d){
  const a=d.arduino, e=d.elbear;
  const fmt = v => v!==null && v!==undefined ? v.toFixed(1) : 'N/A';
  document.getElementById('arduino').innerHTML = `
    <div class="row"><span class="label">LM75A:</span><span class="value">${fmt(a.lm75a_temp)} °C</span></div>
    <div class="row"><span class="label">DHT22 T:</span><span class="value">${fmt(a.dht_temp)} °C</span></div>
    <div class="row"><span class="label">DHT22 H:</span><span class="value">${fmt(a.dht_humidity)} %</span></div>
    <div class="row"><span class="label">BMP280 T:</span><span class="value">${fmt(a.bmp_temp)} °C</span></div>
    <div class="row"><span class="label">BMP280 P:</span><span class="value">${fmt(a.bmp_pressure)} mmHg</span></div>`;
  document.getElementById('elbear').innerHTML = `
    <div class="row"><span class="label">THP80 T:</span><span class="value">${fmt(e.thp_temp)} °C</span></div>
    <div class="row"><span class="label">THP80 H:</span><span class="value">${fmt(e.thp_humidity)} %</span></div>
    <div class="row"><span class="label">THP80 P:</span><span class="value">${fmt(e.thp_pressure)} mmHg</span></div>
    <div class="row"><span class="label">Wind:</span><span class="value">${fmt(e.wind_speed)} m/s</span></div>
    <div class="row"><span class="label">Rain:</span><span class="value">${fmt(e.rainfall)} mm</span></div>
    <div class="row"><span class="label">Light:</span><span class="value">${fmt(e.light_intensity)} lux</span></div>`;
  document.getElementById('ts').textContent = d.last_update || '-';
}
</script></body></html>
"""

@app.route('/')
def index():
    return render_template_string(HTML_TEMPLATE)

@app.route('/api/data')
def get_data():
    return jsonify(weather_data)


# ==================== ЗАПУСК ====================
def main():
    log_message("=== Weather Station Server Starting ===")
    log_message(f"Arduino MAC: {MAC_ARDUINO}")
    log_message(f"Elbear MAC:  {MAC_ELBEAR}")

    arduino_bt = BluetoothClient(MAC_ARDUINO, 'Arduino')
    elbear_bt  = BluetoothClient(MAC_ELBEAR,  'Elbear')

    threading.Thread(target=arduino_bt.read_loop, args=(handle_data,), daemon=True).start()
    threading.Thread(target=elbear_bt.read_loop,  args=(handle_data,), daemon=True).start()
    threading.Thread(target=tcp_server, daemon=True).start()
    threading.Thread(target=lcd_loop, daemon=True).start()

    log_message(f"Web server on http://0.0.0.0:{HTTP_PORT}")
    socketio.run(app, host='0.0.0.0', port=HTTP_PORT, debug=False)


if __name__ == '__main__':
    main()
