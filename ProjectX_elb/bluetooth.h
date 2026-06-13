#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "config.h"
#include "sensors.h"

// Initialize Bluetooth module
void initBluetooth();

// Send sensor data via Bluetooth
void sendSensorData(const SensorData &data);

// Send formatted packet
void sendPacket(const String &packet);

// Build data packet string
String buildDataPacket(const SensorData &data);

#endif // BLUETOOTH_H
