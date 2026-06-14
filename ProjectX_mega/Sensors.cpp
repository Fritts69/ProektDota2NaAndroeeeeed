#include "Sensors.h"

Sensors::Sensors() : dht22(5) {
}

void Sensors::begin() {
    Wire.begin();
    bmp.begin();
}

String Sensors::readData() {
    String s_tmp = String(temperature.readTemperatureC(), 1);
    String s_hmd = String(dht22.getHumidity(), 1);
    String s_prs = String(bmp.readPressure());// * 760.0 / 101325.0, 1);
    return "{'temp': " + s_tmp + ", 'humid': " + s_hmd + ", 'press': " + s_prs + "}";
}
