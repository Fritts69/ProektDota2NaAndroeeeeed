from flask import Flask, jsonify, render_template_string
import sqlite3
from datetime import datetime

app = Flask(__name__)

# HTML шаблон
HTML_TEMPLATE = '''
<!DOCTYPE html>
<html>
<head>
    <title>Погодная станция - Arch Linux</title>
    <meta http-equiv="refresh" content="5">
    <meta charset="UTF-8">
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        h1 {
            color: white;
            text-align: center;
            margin-bottom: 30px;
            font-size: 2.5em;
        }
        .sensor-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .card {
            background: white;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        .card h2 {
            color: #667eea;
            margin-bottom: 15px;
            border-bottom: 2px solid #667eea;
            padding-bottom: 5px;
        }
        .sensor-value {
            font-size: 1.2em;
            margin: 10px 0;
            padding: 8px;
            background: #f5f5f5;
            border-radius: 5px;
        }
        .sensor-label {
            font-weight: bold;
            display: inline-block;
            width: 120px;
        }
        .table-container {
            background: white;
            border-radius: 10px;
            padding: 20px;
            overflow-x: auto;
        }
        table {
            width: 100%;
            border-collapse: collapse;
        }
        th, td {
            padding: 10px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        th {
            background: #667eea;
            color: white;
        }
        tr:hover {
            background: #f5f5f5;
        }
        .update-time {
            color: white;
            text-align: center;
            margin-top: 20px;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌡️ Погодная станция</h1>
        
        <div class="sensor-grid">
            <div class="card">
                <h2>📊 Elbear (Датчики)</h2>
                <div class="sensor-value">
                    <span class="sensor-label">Температура:</span>
                    <span id="temp">{{ latest_elbear.temperature or '--' }} °C</span>
                </div>
                <div class="sensor-value">
                    <span class="sensor-label">Влажность:</span>
                    <span id="hum">{{ latest_elbear.humidity or '--' }} %</span>
                </div>
                <div class="sensor-value">
                    <span class="sensor-label">Давление:</span>
                    <span id="pres">{{ latest_elbear.pressure or '--' }} мм рт.ст.</span>
                </div>
                <div class="sensor-value">
                    <span class="sensor-label">Освещенность:</span>
                    <span id="light">{{ latest_elbear.light or '--' }}</span>
                </div>
            </div>
            
            <div class="card">
                <h2>🎮 Arduino Mega</h2>
                <div class="sensor-value">
                    <span class="sensor-label">CO2:</span>
                    <span id="co2">{{ latest_arduino.co2 or '--' }} ppm</span>
                </div>
                <div class="sensor-value">
                    <span class="sensor-label">Расстояние:</span>
                    <span id="dist">{{ latest_arduino.distance or '--' }} мм</span>
                </div>
                <div class="sensor-value">
                    <span class="sensor-label">Акселерометр X:</span>
                    <span id="ax">{{ latest_arduino.ax or '--' }}</span>
                </div>
                <div class="sensor-value">
                    <span class="sensor-label">Акселерометр Y:</span>
                    <span id="ay">{{ latest_arduino.ay or '--' }}</span>
                </div>
            </div>
        </div>
        
        <div class="table-container">
            <h2>📋 Последние данные</h2>
            <table>
                <thead>
                    <tr>
                        <th>Время</th>
                        <th>Устройство</th>
                        <th>Температура</th>
                        <th>Влажность</th>
                        <th>CO2</th>
                    </tr>
                </thead>
                <tbody>
                    {% for row in recent_data %}
                    <tr>
                        <td>{{ row[0] }}</td>
                        <td>{{ row[1] }}</td>
                        <td>{{ row[2] or '--' }}</td>
                        <td>{{ row[3] or '--' }}</td>
                        <td>{{ row[4] or '--' }}</td>
                    </tr>
                    {% endfor %}
                </tbody>
            </table>
        </div>
        <div class="update-time">
            Обновлено: {{ current_time }}
        </div>
    </div>
</body>
</html>
'''

def get_db_connection():
    """Получение соединения с БД"""
    conn = sqlite3.connect("weather.db")
    conn.row_factory = sqlite3.Row
    return conn

@app.route('/')
def index():
    conn = get_db_connection()
    c = conn.cursor()
    
    # Последние данные от Elbear
    c.execute('''SELECT temperature, humidity, pressure, light 
                 FROM sensor_data 
                 WHERE device = 'Elbear' 
                 ORDER BY timestamp DESC LIMIT 1''')
    latest_elbear = c.fetchone()
    if not latest_elbear:
        latest_elbear = {'temperature': None, 'humidity': None, 'pressure': None, 'light': None}
    
    # Последние данные от Arduino
    c.execute('''SELECT co2, distance, ax, ay, az 
                 FROM sensor_data 
                 WHERE device = 'ArduinoMega' 
                 ORDER BY timestamp DESC LIMIT 1''')
    latest_arduino = c.fetchone()
    if not latest_arduino:
        latest_arduino = {'co2': None, 'distance': None, 'ax': None, 'ay': None, 'az': None}
    
    # Последние 20 записей
    c.execute('''SELECT timestamp, device, temperature, humidity, co2 
                 FROM sensor_data 
                 ORDER BY timestamp DESC LIMIT 20''')
    recent_data = c.fetchall()
    
    conn.close()
    
    return render_template_string(
        HTML_TEMPLATE,
        latest_elbear=latest_elbear,
        latest_arduino=latest_arduino,
        recent_data=recent_data,
        current_time=datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    )

@app.route('/api/data')
def api_data():
    """JSON API для Qt клиента"""
    conn = get_db_connection()
    c = conn.cursor()
    c.execute('''SELECT timestamp, device, temperature, humidity, pressure, 
                        co2, distance, light, sound, flame
                 FROM sensor_data 
                 ORDER BY timestamp DESC LIMIT 50''')
    rows = c.fetchall()
    conn.close()
    
    data = []
    for row in rows:
        data.append({
            'timestamp': row[0],
            'device': row[1],
            'temperature': row[2],
            'humidity': row[3],
            'pressure': row[4],
            'co2': row[5],
            'distance': row[6],
            'light': row[7],
            'sound': row[8],
            'flame': row[9]
        })
    return jsonify(data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=False)
