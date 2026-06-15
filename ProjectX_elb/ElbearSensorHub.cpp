#include "ElbearSensorHub.h"

ElbearSensorHub::ElbearSensorHub() {
}

void ElbearSensorHub::begin() {
    // Инициализация Serial для отладки через USB
    Serial.begin(9600);
    while (!Serial) {
        delay(10);
    }
    
    // Инициализация аппаратного Serial1 для Bluetooth
    Serial1.begin(BLUETOOTH_BAUD);
    
    // Инициализация I2C (без мультиплексора!)
    Wire.begin();
    
    // Вывод в оба порта
    Serial.println("=== Elbear Sensor Hub v1.0 (без TCA9548A) ===");
    Serial.println("Инициализация датчиков...");
    Serial1.println("Elbear Sensor Hub v1.0");

    // --- MGS-THP80 (BME280) - адрес 0x77 ---
    if (bme280.begin(ADDR_BME280)) {
        Serial.println("[OK] MGS-THP80 (BME280) найден на 0x77");
    } else {
        Serial.println("[FAIL] MGS-THP80 не найден");
    }

    // --- MGS-L75 (BH1750) - адрес 0x23 ---
    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println("[OK] MGS-L75 (BH1750) найден на 0x23");
    } else {
        Serial.println("[FAIL] MGS-L75 не найден");
    }

    // --- MGS-D20 (VL53L0X) - адрес 0x29 ---
    if (vl53.begin()) {
        Serial.println("[OK] MGS-D20 (VL53L0X) найден на 0x29");
    } else {
        Serial.println("[FAIL] MGS-D20 не найден");
    }

    // --- MGS-CLM60 (APDS-9960) - адрес 0x39 ---
    if (apds.begin()) {
        Serial.println("[OK] MGS-CLM60 (APDS-9960) найден на 0x39");
        apds.enableProximity(true);
        apds.enableColor(true);
    } else {
        Serial.println("[FAIL] MGS-CLM60 не найден");
    }

    // --- MGS-CO30 (SGP30) - адрес 0x58 ---
    if (sgp30.begin()) {
        Serial.println("[OK] MGS-CO30 (SGP30) найден на 0x58");
        sgp30.setIAQBaseline(0x8973, 0x8AAE);
    } else {
        Serial.println("[FAIL] MGS-CO30 не найден");
    }

    // --- MGS-A6 (MPU6050) - адрес 0x69 ---
    if (mpu.begin()) {
        Serial.println("[OK] MGS-A6 (MPU6050) найден на 0x69");
        mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
        mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    } else {
        Serial.println("[FAIL] MGS-A6 не найден");
    }

    // --- MGS-WT1 (протечка) ---
    pinMode(WT1_PIN, INPUT_PULLUP);
    Serial.println("[OK] MGS-WT1 (Water Leak) настроен на D3");

    // --- MGS-SND504 (звук) ---
    pinMode(SND_PIN, INPUT);
    Serial.println("[OK] MGS-SND504 (Sound) настроен на A0");

    // --- MGS-FR403 (пламя) ---
    pinMode(FLAME_PIN, INPUT);
    Serial.println("[OK] MGS-FR403 (Flame) настроен на A1");

    Serial.println("===============================");
    Serial.println("Все датчики инициализированы!");
    Serial.println("Готов к работе!");
    Serial1.println("Все датчики инициализированы");
}

void ElbearSensorHub::readAllSensors() {
    String data = "{";

    // 1. MGS-THP80 (BME280) - температура, влажность, давление
    data += "\"THP80_temp\": " + String(bme280.readTemperature(), 1) + ",";
    data += "\"THP80_hum\": " + String(bme280.readHumidity(), 1) + ",";
    data += "\"THP80_press\": " + String(bme280.readPressure() / 133.322, 1) + ",";

    // 2. MGS-L75 (BH1750) - освещённость
    data += "\"L75_lux\": " + String(lightMeter.readLightLevel(), 1) + ",";

    // 3. MGS-FR403 - датчик пламени (аналоговое значение)
    data += "\"FR403_flame\": " + String(analogRead(FLAME_PIN)) + ",";

    // 4. MGS-SND504 - датчик звука
    data += "\"SND504_sound\": " + String(analogRead(SND_PIN)) + ",";

    // 5. MGS-WT1 - протечка воды
    data += "\"WT1_leak\": " + String(digitalRead(WT1_PIN) == LOW ? "\"WET\"" : "\"DRY\"") + ",";

    // 6. MGS-CLM60 (APDS-9960) - цвет и приближение
    if (apds.colorDataReady()) {
        uint16_t r, g, b, c;
        apds.getColorData(&r, &g, &b, &c);
        data += "\"CLM60_red\": " + String(r) + ",";
        data += "\"CLM60_green\": " + String(g) + ",";
        data += "\"CLM60_blue\": " + String(b) + ",";
        data += "\"CLM60_clear\": " + String(c) + ",";
    }
    data += "\"CLM60_proximity\": " + String(apds.readProximity()) + ",";

    // 7. MGS-A6 (MPU6050) - акселерометр и гироскоп
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    data += "\"A6_accel_x\": " + String(a.acceleration.x, 2) + ",";
    data += "\"A6_accel_y\": " + String(a.acceleration.y, 2) + ",";
    data += "\"A6_accel_z\": " + String(a.acceleration.z, 2) + ",";
    data += "\"A6_gyro_x\": " + String(g.gyro.x, 2) + ",";
    data += "\"A6_gyro_y\": " + String(g.gyro.y, 2) + ",";
    data += "\"A6_gyro_z\": " + String(g.gyro.z, 2) + ",";

    // 8. MGS-CO30 (SGP30) - eCO2 и TVOC
    if (sgp30.IAQmeasure()) {
        data += "\"CO30_eco2\": " + String(sgp30.eCO2) + ",";
        data += "\"CO30_tvoc\": " + String(sgp30.TVOC) + ",";
    } else {
        data += "\"CO30_eco2\": 0,";
        data += "\"CO30_tvoc\": 0,";
    }

    // 9. MGS-D20 (VL53L0X) - расстояние
    VL53L0X_RangingMeasurementData_t measure;
    vl53.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
        data += "\"D20_distance\": " + String(measure.RangeMilliMeter);
    } else {
        data += "\"D20_distance\": -1";
    }
    data += "}";

    // Отправляем JSON через аппаратный Serial1 (Bluetooth)
    Serial1.println(data);
    
    // Дублируем в Serial Monitor для отладки
    Serial.println(data);
}
