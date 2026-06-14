#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Temperature_LM75_Derived.h>
#include <DHT22.h>
#include <GyverBME280.h>

class Sensors {
public:
    Sensors();
    void begin();
    String readData();

private:
    Generic_LM75 temperature;
    DHT22 dht22;
    GyverBME280 bmp;
};

#endif // SENSORS_H
