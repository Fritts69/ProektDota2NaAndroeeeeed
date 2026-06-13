#include "sensors.h"

bool initSensors() {
    bool allSuccess = true;
    
    Serial.println("Initializing sensors...");
    
    if (!initMGS_THP80()) {
        Serial.println("Failed to init MGS-THP80");
        allSuccess = false;
    } else {
        Serial.println("MGS-THP80 initialized");
    }
    
    if (!initMGS_WT1()) {
        Serial.println("Failed to init MGS-WT1");
        allSuccess = false;
    } else {
        Serial.println("MGS-WT1 initialized");
    }
    
    if (!initMGS_CLM60()) {
        Serial.println("Failed to init MGS-CLM60");
        allSuccess = false;
    } else {
        Serial.println("MGS-CLM60 initialized");
    }
    
    if (!initMGS_FR403()) {
        Serial.println("Failed to init MGS-FR403");
        allSuccess = false;
    } else {
        Serial.println("MGS-FR403 initialized");
    }
    
    if (!initMGS_L75()) {
        Serial.println("Failed to init MGS-L75");
        allSuccess = false;
    } else {
        Serial.println("MGS-L75 initialized");
    }
    
    if (!initMGS_SND504()) {
        Serial.println("Failed to init MGS-SND504");
        allSuccess = false;
    } else {
        Serial.println("MGS-SND504 initialized");
    }
    
    if (!initMGS_A6()) {
        Serial.println("Failed to init MGS-A6");
        allSuccess = false;
    } else {
        Serial.println("MGS-A6 initialized");
    }
    
    if (!initMGS_CO30()) {
        Serial.println("Failed to init MGS-CO30");
        allSuccess = false;
    } else {
        Serial.println("MGS-CO30 initialized");
    }
    
    if (!initMGS_D20()) {
        Serial.println("Failed to init MGS-D20");
        allSuccess = false;
    } else {
        Serial.println("MGS-D20 initialized");
    }
    
    return allSuccess;
}

bool initMGS_THP80() {
    return writeByteData(MGS_THP80_ADDR, 0x00, 0x00);
}

bool initMGS_WT1() {
    return writeByteData(MGS_WT1_ADDR, 0x00, 0x00);
}

bool initMGS_CLM60() {
    return writeByteData(MGS_CLM60_ADDR, 0x00, 0x00);
}

bool initMGS_FR403() {
    return writeByteData(MGS_FR403_ADDR, 0x00, 0x00);
}

bool initMGS_L75() {
    return writeByteData(MGS_L75_ADDR, 0x00, 0x00);
}

bool initMGS_SND504() {
    return writeByteData(MGS_SND504_ADDR, 0x00, 0x00);
}

bool initMGS_A6() {
    return writeByteData(MGS_A6_ADDR, 0x00, 0x00);
}

bool initMGS_CO30() {
    return writeByteData(MGS_CO30_ADDR, 0x00, 0x00);
}

bool initMGS_D20() {
    return writeByteData(MGS_D20_ADDR, 0x00, 0x00);
}

void readAllSensors(SensorData &data) {
    readMGS_THP80(data);
    readMGS_WT1(data);
    readMGS_CLM60(data);
    readMGS_FR403(data);
    readMGS_L75(data);
    readMGS_SND504(data);
    readMGS_A6(data);
    readMGS_CO30(data);
    readMGS_D20(data);
}

void readMGS_THP80(SensorData &data) {
    Wire.beginTransmission(MGS_THP80_ADDR);
    Wire.write(0x00); // Data register
    if (Wire.endTransmission() != 0) {
        data.thp_error = true;
        data.thp_temp = ERROR_VALUE;
        data.thp_humidity = ERROR_VALUE;
        data.thp_pressure = ERROR_VALUE;
        return;
    }
    
    Wire.requestFrom(MGS_THP80_ADDR, 6);
    if (Wire.available() >= 6) {
        uint8_t buf[6];
        for (int i = 0; i < 6; i++) {
            buf[i] = Wire.read();
        }
        
        data.thp_temp = (buf[0] << 8 | buf[1]) * 0.01;
        data.thp_humidity = (buf[2] << 8 | buf[3]) * 0.01;
        data.thp_pressure = (buf[4] << 8 | buf[5]) * 0.01;
        data.thp_error = false;
    } else {
        data.thp_error = true;
        data.thp_temp = ERROR_VALUE;
        data.thp_humidity = ERROR_VALUE;
        data.thp_pressure = ERROR_VALUE;
    }
}

void readMGS_WT1(SensorData &data) {
    Wire.beginTransmission(MGS_WT1_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        data.wt1_error = true;
        data.wind_speed = ERROR_VALUE;
        data.wind_direction = ERROR_VALUE;
        return;
    }
    
    Wire.requestFrom(MGS_WT1_ADDR, 4);
    if (Wire.available() >= 4) {
        uint8_t buf[4];
        for (int i = 0; i < 4; i++) {
            buf[i] = Wire.read();
        }
        
        data.wind_speed = (buf[0] << 8 | buf[1]) * 0.1;
        data.wind_direction = (buf[2] << 8 | buf[3]) * 0.1;
        data.wt1_error = false;
    } else {
        data.wt1_error = true;
        data.wind_speed = ERROR_VALUE;
        data.wind_direction = ERROR_VALUE;
    }
}

void readMGS_CLM60(SensorData &data) {
    Wire.beginTransmission(MGS_CLM60_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        data.clm60_error = true;
        data.rainfall = ERROR_VALUE;
        return;
    }
    
    Wire.requestFrom(MGS_CLM60_ADDR, 2);
    if (Wire.available() >= 2) {
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        data.rainfall = (msb << 8 | lsb) * 0.1;
        data.clm60_error = false;
    } else {
        data.clm60_error = true;
        data.rainfall = ERROR_VALUE;
    }
}

void readMGS_FR403(SensorData &data) {
    Wire.beginTransmission(MGS_FR403_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        data.fr403_error = true;
        data.radiation = ERROR_VALUE;
        return;
    }
    
    Wire.requestFrom(MGS_FR403_ADDR, 2);
    if (Wire.available() >= 2) {
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        data.radiation = (msb << 8 | lsb) * 0.01;
        data.fr403_error = false;
    } else {
        data.fr403_error = true;
        data.radiation = ERROR_VALUE;
    }
}

void readMGS_L75(SensorData &data) {
    Wire.beginTransmission(MGS_L75_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        data.l75_error = true;
        data.light_intensity = ERROR_VALUE;
        return;
    }
    
    Wire.requestFrom(MGS_L75_ADDR, 2);
    if (Wire.available() >= 2) {
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        data.light_intensity = (msb << 8 | lsb);
        data.l75_error = false;
    } else {
        data.l75_error = true;
        data.light_intensity = ERROR_VALUE;
    }
}

void readMGS_SND504(SensorData &data) {
    Wire.beginTransmission(MGS_SND504_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        data.snd504_error = true;
        data.sound_level = ERROR_VALUE;
        return;
    }
    
    Wire.requestFrom(MGS_SND504_ADDR, 2);
    if (Wire.available() >= 2) {
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        data.sound_level = (msb << 8 | lsb) * 0.1;
        data.snd504_error = false;
    } else {
        data.snd504_error = true;
        data.sound_level = ERROR_VALUE;
    }
}

void readMGS_A6(SensorData &data) {
    Wire.beginTransmission(MGS_A6_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        data.a6_error = true;
        data.gas_level = ERROR_VALUE;
        return;
    }
    
    Wire.requestFrom(MGS_A6_ADDR, 2);
    if (Wire.available() >= 2) {
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        data.gas_level = (msb << 8 | lsb) * 0.01;
        data.a6_error = false;
    } else {
        data.a6_error = true;
        data.gas_level = ERROR_VALUE;
    }
}

void readMGS_CO30(SensorData &data) {
    Wire.beginTransmission(MGS_CO30_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        data.co30_error = true;
        data.co_level = ERROR_VALUE;
        return;
    }
    
    Wire.requestFrom(MGS_CO30_ADDR, 2);
    if (Wire.available() >= 2) {
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        data.co_level = (msb << 8 | lsb) * 0.01;
        data.co30_error = false;
    } else {
        data.co30_error = true;
        data.co_level = ERROR_VALUE;
    }
}

void readMGS_D20(SensorData &data) {
    Wire.beginTransmission(MGS_D20_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        data.d20_error = true;
        data.distance = ERROR_VALUE;
        return;
    }
    
    Wire.requestFrom(MGS_D20_ADDR, 2);
    if (Wire.available() >= 2) {
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        data.distance = (msb << 8 | lsb) * 0.1;
        data.d20_error = false;
    } else {
        data.d20_error = true;
        data.distance = ERROR_VALUE;
    }
}

// Helper functions
uint16_t readWordData(uint8_t addr, uint8_t reg) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return 0xFFFF;
    }
    
    Wire.requestFrom(addr, 2);
    if (Wire.available() >= 2) {
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        return (msb << 8) | lsb;
    }
    
    return 0xFFFF;
}

bool writeByteData(uint8_t addr, uint8_t reg, uint8_t value) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}
