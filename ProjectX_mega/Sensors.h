#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Temperature_LM75_Derived.h>
#include <DHT22.h>
//#include <Adafruit_BMP280.h>
#include <GyverBME280.h>

#define BMP_SCK  (8)  // scl
#define BMP_MISO (12) // sdo
#define BMP_MOSI (9) // sda
#define BMP_CS   (11) // csb

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
