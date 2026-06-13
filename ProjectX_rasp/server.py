# server.py
from flask import Flask, jsonify, render_template
from flask_socketio import SocketIO
import json
import threading
import socket

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

weather_data = {
    'arduino': {},
    'elbear': {},
    'timestamp': None
}

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/data')
def get_data():
    return jsonify(weather_data)

@socketio.on('connect')
def handle_connect():
    print('Client connected')
    socketio.emit('data_update', weather_data)

def tcp_server():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('0.0.0.0', 5001))
    sock.listen(1)
    print("TCP server listening on port 5001")
    
    while True:
        conn, addr = sock.accept()
        try:
            data = conn.recv(1024).decode('utf-8')
            if data:
                parsed = json.loads(data)
                global weather_data
                weather_data = parsed
                socketio.emit('data_update', weather_data)
        except Exception as e:
            print(f"Error: {e}")
        finally:
            conn.close()

if __name__ == '__main__':
    tcp_thread = threading.Thread(target=tcp_server, daemon=True)
    tcp_thread.start()
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
