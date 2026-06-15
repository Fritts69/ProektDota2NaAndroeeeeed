#ifndef ELBEAR_SENSOR_HUB_H
#define ELBEAR_SENSOR_HUB_H

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"
#include "Adafruit_APDS9960.h"
#include "Adafruit_VL53L0X.h"
#include "Adafruit_BME280.h"
#include "BH1750.h"
#include "Adafruit_MPU6050.h"

// ========== I2C Адреса датчиков (без мультиплексора!) ==========
// Все адреса уникальны — конфликтов нет
#define ADDR_BME280   0x77  // MGS-THP80 (BME280) - температура/влажность/давление
#define ADDR_BH1750   0x23  // MGS-L75 (BH1750) - освещённость
#define ADDR_VL53L0X  0x29  // MGS-D20 (VL53L0X) - лазерный дальномер
#define ADDR_APDS9960 0x39  // MGS-CLM60 (APDS-9960) - цвет/жесты/приближение
#define ADDR_SGP30    0x58  // MGS-CO30 (SGP30) - качество воздуха (eCO2/TVOC)
#define ADDR_MPU6050  0x69  // MGS-A6 (MPU6050) - акселерометр/гироскоп

// ========== Пины ==========
#define WT1_PIN 3     // MGS-WT1 (протечка) - цифровой
#define SND_PIN A0    // MGS-SND504 (звук) - аналоговый
#define FLAME_PIN A1  // MGS-FR403 (пламя) - аналоговый
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
    BH1750 lightMeter;
    Adafruit_MPU6050 mpu;
};

#endif // ELBEAR_SENSOR_HUB_H
