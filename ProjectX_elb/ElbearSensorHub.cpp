#include "ElbearSensorHub.h"

ElbearSensorHub::ElbearSensorHub() {
    bh1750_lux = 0;
    bh1750_initialized = false;
}

void ElbearSensorHub::tcaselect(uint8_t channel) {
    if (channel > 7) return;
    Wire.beginTransmission(TCAADDR);
    Wire.write(1 << channel);
    Wire.endTransmission();
}

// ========== Прямая работа с BH1750 ==========
void ElbearSensorHub::initBH1750() {
    tcaselect(0);  // BH1750 на канале 0
    Wire.beginTransmission(ADDR_BH1750);
    Wire.write(0x10);  // Continuous H-resolution mode
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
        bh1750_initialized = true;
        Serial.println("[OK] BH1750 инициализирован");
    } else {
        bh1750_initialized = false;
        Serial.println("[FAIL] BH1750 не отвечает");
    }
}

float ElbearSensorHub::readBH1750() {
    if (!bh1750_initialized) {
        return -1;
    }
    
    tcaselect(0);  // BH1750 на канале 0
    Wire.requestFrom(ADDR_BH1750, (uint8_t)2);
    
    if (Wire.available() == 2) {
        uint16_t raw = (Wire.read() << 8) | Wire.read();
        float lux = raw / 1.2;
        return lux;
    }
    
    return -1;
}

// ========== Инициализация всех датчиков ==========
void ElbearSensorHub::begin() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    Serial1.begin(BLUETOOTH_BAUD);
    Wire.begin();
    
    Serial.println("=== Elbear Sensor Hub v1.0 ===");
    Serial.println("Инициализация датчиков...");
    Serial1.println("Elbear Sensor Hub v1.0");
    
    // --- MGS-CO30 (SGP30) на канале 5 ---
    tcaselect(5);
    if (sgp30.begin()) {
        Serial.println("[OK] SGP30 найден");
        sgp30.setIAQBaseline(0x8973, 0x8AAE);
    } else {
        Serial.println("[FAIL] SGP30 не найден");
    }
    
    // --- MGS-CLM60 (APDS-9960) на канале 3 ---
    tcaselect(3);
    if (apds.begin()) {
        Serial.println("[OK] APDS-9960 найден");
        apds.enableProximity(true);
        apds.enableColor(true);
    } else {
        Serial.println("[FAIL] APDS-9960 не найден");
    }
    
    // --- MGS-D20 (VL53L0X) на канале 6 ---
    tcaselect(6);
    if (vl53.begin()) {
        Serial.println("[OK] VL53L0X найден");
    } else {
        Serial.println("[FAIL] VL53L0X не найден");
    }
    
    // --- MGS-THP80 (BME280) на канале 0 ---
    tcaselect(0);
    if (bme280.begin(ADDR_BME280)) {
        Serial.println("[OK] BME280 найден");
    } else {
        Serial.println("[FAIL] BME280 не найден");
    }
    
    // --- MGS-L75 (BH1750) на канале 0 ---
    initBH1750();
    
    // --- MGS-A6 (MPU6050) на канале 4 ---
    tcaselect(4);
    if (mpu.begin()) {
        Serial.println("[OK] MPU6050 найден");
        mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
        mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    } else {
        Serial.println("[FAIL] MPU6050 не найден");
    }
    
    // --- Цифровые/аналоговые датчики ---
    pinMode(WT1_PIN, INPUT_PULLUP);
    pinMode(SND_PIN, INPUT);
    pinMode(FLAME_PIN, INPUT);
    
    Serial.println("[OK] Цифровые датчики настроены");
    Serial.println("===============================");
    Serial.println("Все датчики инициализированы!");
    Serial.println("Готов к работе!");
    Serial1.println("Все датчики инициализированы");
}

// ========== Чтение всех датчиков ==========
void ElbearSensorHub::readAllSensors() {
    String data = "{";
    
    // 1. MGS-THP80 (BME280) - температура, влажность, давление
    tcaselect(0);
    float temp = bme280.readTemperature();
    float hum = bme280.readHumidity();
    float press = bme280.readPressure() / 133.322;  // в мм рт.ст.
    
    if (isnan(temp)) temp = -999;
    if (isnan(hum)) hum = -999;
    if (isnan(press)) press = -999;
    
    data += "\"THP80_temp\": " + String(temp, 1) + ",";
    data += "\"THP80_hum\": " + String(hum, 1) + ",";
    data += "\"THP80_press\": " + String(press, 1) + ",";
    
    // 2. MGS-L75 (BH1750) - освещенность
    float lux = readBH1750();
    data += "\"L75_lux\": " + String(lux, 1) + ",";
    
    // 3. MGS-FR403 - датчик пламени
    data += "\"FR403_flame\": " + String(analogRead(FLAME_PIN)) + ",";
    
    // 4. MGS-SND504 - датчик звука
    data += "\"SND504_sound\": " + String(analogRead(SND_PIN)) + ",";
    
    // 5. MGS-WT1 - протечка воды
    String leakStatus = (digitalRead(WT1_PIN) == LOW) ? "WET" : "DRY";
    data += "\"WT1_leak\": \"" + leakStatus + "\",";
    
    // 6. MGS-CLM60 (APDS-9960) - цвет и приближение
    tcaselect(3);
    if (apds.colorDataReady()) {
        uint16_t r, g, b, c;
        apds.getColorData(&r, &g, &b, &c);
        data += "\"CLM60_red\": " + String(r) + ",";
        data += "\"CLM60_green\": " + String(g) + ",";
        data += "\"CLM60_blue\": " + String(b) + ",";
        data += "\"CLM60_clear\": " + String(c) + ",";
    } else {
        data += "\"CLM60_red\": 0,\"CLM60_green\": 0,\"CLM60_blue\": 0,\"CLM60_clear\": 0,";
    }
    data += "\"CLM60_proximity\": " + String(apds.readProximity()) + ",";
    
    // 7. MGS-A6 (MPU6050) - акселерометр и гироскоп
    tcaselect(4);
    sensors_event_t a, g, temp_event;
    mpu.getEvent(&a, &g, &temp_event);
    
    data += "\"A6_accel_x\": " + String(a.acceleration.x, 2) + ",";
    data += "\"A6_accel_y\": " + String(a.acceleration.y, 2) + ",";
    data += "\"A6_accel_z\": " + String(a.acceleration.z, 2) + ",";
    data += "\"A6_gyro_x\": " + String(g.gyro.x, 2) + ",";
    data += "\"A6_gyro_y\": " + String(g.gyro.y, 2) + ",";
    data += "\"A6_gyro_z\": " + String(g.gyro.z, 2) + ",";
    
    // 8. MGS-CO30 (SGP30) - eCO2 и TVOC
    tcaselect(5);
    if (sgp30.IAQmeasure()) {
        data += "\"CO30_eco2\": " + String(sgp30.eCO2) + ",";
        data += "\"CO30_tvoc\": " + String(sgp30.TVOC) + ",";
    } else {
        data += "\"CO30_eco2\": 0,\"CO30_tvoc\": 0,";
    }
    
    // 9. MGS-D20 (VL53L0X) - расстояние
    tcaselect(6);
    VL53L0X_RangingMeasurementData_t measure;
    vl53.rangingTest(&measure, false);
    
    if (measure.RangeStatus != 4) {
        data += "\"D20_distance\": " + String(measure.RangeMilliMeter);
    } else {
        data += "\"D20_distance\": -1";
    }
    
    data += "}";
    
    // Отправляем через Bluetooth (Serial1)
    Serial1.println(data);
    
    // Дублируем в Serial Monitor для отладки
    Serial.println(data);
}
