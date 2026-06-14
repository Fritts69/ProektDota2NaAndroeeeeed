#include "Sensors.h"

Sensors::Sensors() : dht22(5) {
}

void Sensors::begin() {
    Wire.begin();
    bmp.begin();
}

String Sensors::readData() {
    // Считываем данные
    float tmp_lm75 = temperature.readTemperatureC();
    float hum_dht = dht22.getHumidity();
    float tmp_dht = dht22.getTemperature();
    
    // BME280 возвращает давление в Паскалях. Переводим в мм.рт.ст. (mmHg)
    float press_bmp = bmp.readPressure();
    float tmp_bmp = bmp.readTemperature();

    // Формируем пакет для Raspberry Pi сервера:
    // ARDUINO|LM75A_TEMP|DHT_TEMP|DHT_HUM|BMP_TEMP|BMP_PRESS|TIMESTAMP
    String packet = "ARDUINO|";
    packet += String(tmp_lm75, 1) + "|";
    packet += String(hum_dht, 1) + "|";
    packet += String(press_bmp, 1) + "|";
    packet += "0"; // Плейсхолдер для timestamp (сервер подставит свой)

    return packet;
}
