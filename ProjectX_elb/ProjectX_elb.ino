#include <Wire.h>
#include "config.h"
#include "sensors.h"
#include "bluetooth.h"

SensorData sensorData;
unsigned long previousMillis = 0;

void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for Serial Monitor
    }
    
    Serial.println("\n=== Elbear Weather Station ===");
    
    // Initialize I2C
    Wire.begin();
    
    // Initialize sensors
    initSensors();
    
    // Initialize Bluetooth
    initBluetooth();
    
    Serial.println("System ready!");
    Serial.println();
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Check if it's time to read sensors and send data
    if (currentMillis - previousMillis >= DATA_INTERVAL) {
        previousMillis = currentMillis;
        
        // Read all sensors
        readAllSensors(sensorData);
        
        // Send data via Bluetooth
        sendSensorData(sensorData);
        
        // Print debug info (optional)
        printDebugInfo();
    }
}

void printDebugInfo() {
    Serial.println("--- Sensor Data ---");
    Serial.print("THP80 - T: ");
    Serial.print(sensorData.thp_temp);
    Serial.print(" H: ");
    Serial.print(sensorData.thp_humidity);
    Serial.print(" P: ");
    Serial.println(sensorData.thp_pressure);
    
    Serial.print("Wind - Speed: ");
    Serial.print(sensorData.wind_speed);
    Serial.print(" Dir: ");
    Serial.println(sensorData.wind_direction);
    
    Serial.print("Rain: ");
    Serial.println(sensorData.rainfall);
    Serial.println();
}
