#!/bin/bash

cd /home/alarm/gitclone/ProjectX/ProjectX_rasp
source ./venv/bin/activate

# Запуск Bluetooth приёмника
python3 bluetooth.py &
RECEIVER_PID=$!

# Запуск веб-сервера
python3 server.py &
WEB_PID=$!

# Запуск LCD дисплея
python3 display.py &
LCD_PID=$!

# Запуск авто-подключения Bluetooth
./autoconnect.sh &
BLUETOOTH_PID=$!

echo "Все сервисы запущены"
echo "Приёмник PID: $RECEIVER_PID"
echo "Веб-сервер PID: $WEB_PID"
echo "LCD дисплей PID: $LCD_PID"
echo "Bluetooth авто-подключение PID: $BLUETOOTH_PID"

# Ожидание завершения
wait
