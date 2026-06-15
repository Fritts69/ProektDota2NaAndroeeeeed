#ifndef ELBEAR_SENSOR_HUB_H
#define ELBEAR_SENSOR_HUB_H

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"
#include "Adafruit_APDS9960.h"
#include "Adafruit_VL53L0X.h"
#include "Adafruit_BME280.h"
#include "Adafruit_MPU6050.h"

// ========== I2C Адреса ==========
#define TCAADDR 0x70
#define ADDR_SGP30 0x58
#define ADDR_APDS9960 0x39
#define ADDR_VL53L0X 0x29
#define ADDR_BME280 0x76
#define ADDR_BH1750 0x23
#define ADDR_MPU6050 0x68

// ========== Пины ==========
#define WT1_PIN 3
#define SND_PIN A0
#define FLAME_PIN A1

// ========== Скорость Bluetooth ==========
#define BLUETOOTH_BAUD 9600

class ElbearSensorHub {
public:
    ElbearSensorHub();
    void begin();
    void readAllSensors();

private:
    Adafruit_SGP30 sgp30;
    Adafruit_APDS9960 apds;
    Adafruit_VL53L0X vl53;
    Adafruit_BME280 bme280;
    Adafruit_MPU6050 mpu;
    
    // BH1750 - прямая работа через Wire (библиотека несовместима с MIK32)
    float bh1750_lux;
    bool bh1750_initialized;
    
    void tcaselect(uint8_t channel);
    float readBH1750();
    void initBH1750();
};

#endif
