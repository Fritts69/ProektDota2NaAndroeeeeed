#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// Structure to hold all sensor data
struct SensorData {
    // MGS-THP80: Temperature, Humidity, Pressure
    float thp_temp;
    float thp_humidity;
    float thp_pressure;
    bool thp_error;
    
    // MGS-WT1: Wind speed, direction
    float wind_speed;
    float wind_direction;
    bool wt1_error;
    
    // MGS-CLM60: Rainfall
    float rainfall;
    bool clm60_error;
    
    // MGS-FR403: Radiation
    float radiation;
    bool fr403_error;
    
    // MGS-L75: Light intensity
    float light_intensity;
    bool l75_error;
    
    // MGS-SND504: Sound level
    float sound_level;
    bool snd504_error;
    
    // MGS-A6: Gas (air quality)
    float gas_level;
    bool a6_error;
    
    // MGS-CO30: CO level
    float co_level;
    bool co30_error;
    
    // MGS-D20: Distance
    float distance;
    bool d20_error;
};

// Initialize all sensors
bool initSensors();

// Initialize individual sensors
bool initMGS_THP80();
bool initMGS_WT1();
bool initMGS_CLM60();
bool initMGS_FR403();
bool initMGS_L75();
bool initMGS_SND504();
bool initMGS_A6();
bool initMGS_CO30();
bool initMGS_D20();

// Read data from sensors
void readAllSensors(SensorData &data);

// Read individual sensors
void readMGS_THP80(SensorData &data);
void readMGS_WT1(SensorData &data);
void readMGS_CLM60(SensorData &data);
void readMGS_FR403(SensorData &data);
void readMGS_L75(SensorData &data);
void readMGS_SND504(SensorData &data);
void readMGS_A6(SensorData &data);
void readMGS_CO30(SensorData &data);
void readMGS_D20(SensorData &data);

// Helper functions
uint16_t readWordData(uint8_t addr, uint8_t reg);
bool writeByteData(uint8_t addr, uint8_t reg, uint8_t value);

#endif // SENSORS_H
