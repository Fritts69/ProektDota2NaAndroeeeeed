#include "bluetooth.h"

SoftwareSerial bluetooth(BT_RX_PIN, BT_TX_PIN);

void initBluetooth() {
    bluetooth.begin(BT_BAUD_RATE);
    Serial.println("Bluetooth initialized");
    delay(1000);
    bluetooth.println("ELBEAR_START");
}

void sendSensorData(const SensorData &data) {
    String packet = buildDataPacket(data);
    sendPacket(packet);
}

void sendPacket(const String &packet) {
    bluetooth.println(packet);
    Serial.println(packet);
}

String buildDataPacket(const SensorData &data) {
    unsigned long timestamp = millis();
    
    // Format: ELBEAR|THP_TEMP|THP_HUM|THP_PRESS|WIND_SPD|WIND_DIR|RAIN|RAD|LIGHT|SND|GAS|CO|DIST|TIME
    String packet = "ELBEAR|";
    
    // THP80 data
    packet += (data.thp_error ? String(ERROR_VALUE, 1) : String(data.thp_temp, 1)) + "|";
    packet += (data.thp_error ? String(ERROR_VALUE, 1) : String(data.thp_humidity, 1)) + "|";
    packet += (data.thp_error ? String(ERROR_VALUE, 1) : String(data.thp_pressure, 1)) + "|";
    
    // Wind data
    packet += (data.wt1_error ? String(ERROR_VALUE, 1) : String(data.wind_speed, 1)) + "|";
    packet += (data.wt1_error ? String(ERROR_VALUE, 1) : String(data.wind_direction, 1)) + "|";
    
    // Rainfall
    packet += (data.clm60_error ? String(ERROR_VALUE, 1) : String(data.rainfall, 1)) + "|";
    
    // Radiation
    packet += (data.fr403_error ? String(ERROR_VALUE, 2) : String(data.radiation, 2)) + "|";
    
    // Light
    packet += (data.l75_error ? String(ERROR_VALUE, 1) : String(data.light_intensity, 1)) + "|";
    
    // Sound
    packet += (data.snd504_error ? String(ERROR_VALUE, 1) : String(data.sound_level, 1)) + "|";
    
    // Gas
    packet += (data.a6_error ? String(ERROR_VALUE, 2) : String(data.gas_level, 2)) + "|";
    
    // CO
    packet += (data.co30_error ? String(ERROR_VALUE, 2) : String(data.co_level, 2)) + "|";
    
    // Distance
    packet += (data.d20_error ? String(ERROR_VALUE, 1) : String(data.distance, 1)) + "|";
    
    // Timestamp
    packet += String(timestamp);
    
    return packet;
}
