/*
 * Elbear Sensor Hub
 * Собирает данные со всех MGS-датчиков и отправляет по Bluetooth
 * Формат отправки: JSON
 */

#include <Wire.h>
#include <SoftwareSerial.h>

// ========== I2C Адреса датчиков ==========
#define TCAADDR 0x70          // Адрес I2C мультиплексора TCA9548A
#define ADDR_SGP30 0x58       // MGS-CO30 (SGP30)
#define ADDR_APDS9960 0x39    // MGS-CLM60 (APDS-9960)
#define ADDR_VL53L0X 0x29     // MGS-D20 (VL53L0X)
#define ADDR_BME280 0x76      // MGS-THP80 (BME280)
#define ADDR_BH1750 0x23      // MGS-L75 (BH1750)
#define ADDR_PCA9634 0x62     // MGL-RGB3 (PCA9634) - по умолчанию
#define ADDR_INMP504 0x28     // MGS-SND504 (INMP504) - зависит от модуля
#define ADDR_TSL25403 0x39    // MGS-FR403 (датчик пламени) - может конфликтовать с APDS9960

// ========== Пины для Bluetooth ==========
// Используем SoftwareSerial на свободных пинах
// (пины 0 и 1 оставляем для прошивки и отладки)
#define BT_RX 10   // К TX модуля HC-05
#define BT_TX 11   // К RX модуля HC-05 (через делитель напряжения!)
SoftwareSerial bluetooth(BT_RX, BT_TX); // RX, TX

// ========== Специальные библиотеки ==========
// Для MGS-CO30 (SGP30)
#include "Adafruit_SGP30.h"
Adafruit_SGP30 sgp30;

// Для MGS-CLM60 (APDS-9960) - датчик цвета/жестов
#include "Adafruit_APDS9960.h"
Adafruit_APDS9960 apds;

// Для MGS-D20 (VL53L0X) - лазерный дальномер
#include "Adafruit_VL53L0X.h"
Adafruit_VL53L0X vl53 = Adafruit_VL53L0X();

// Для MGS-THP80 (BME280)
#include "Adafruit_BME280.h"
Adafruit_BME280 bme280;

// Для MGS-L75 (BH1750) - датчик освещенности
#include "BH1750.h"
BH1750 lightMeter;

// Для MGS-WT1 - датчик протечки воды (простой цифровой выход)
#define WT1_PIN 3    // Выход датчика протечки на цифровой пин 3

// Для MGS-A6 - 6-осевой гироскоп/акселерометр
// Рекомендуется MPU6050 или аналогичный
#include "Adafruit_MPU6050.h"
Adafruit_MPU6050 mpu;

// Для MGS-SND504 - датчик звука (аналоговый)
#define SND_PIN A0   // Аналоговый выход микрофона

// Для MGS-FR403 - датчик пламени (аналоговый)
#define FLAME_PIN A1

// Для MGL-RGB3 - LED модуль (будет управляться отдельно, не опрашивается)

// ========== Функция выбора канала мультиплексора ==========
void tcaselect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// ========== Инициализация всех датчиков ==========
void initSensors() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  Wire.begin();
  
  bluetooth.println("Elbear Sensor Hub v1.0");
  Serial.println("Инициализация датчиков...");

  // --- MGS-CO30 (SGP30) на канале 5 ---
  tcaselect(5);
  if (sgp30.begin()) {
    Serial.println("[OK] MGS-CO30 (SGP30) найден");
    sgp30.setIAQBaseline(0x8973, 0x8AAE); // Базовые калибровочные значения
  } else {
    Serial.println("[FAIL] MGS-CO30 не найден");
  }

  // --- MGS-CLM60 (APDS-9960) на канале 3 ---
  tcaselect(3);
  if (apds.begin()) {
    Serial.println("[OK] MGS-CLM60 (APDS-9960) найден");
    apds.enableProximity(true);
    apds.enableColor(true);
  } else {
    Serial.println("[FAIL] MGS-CLM60 не найден");
  }

  // --- MGS-D20 (VL53L0X) на канале 5 (другой канал, т.к. адрес 0x29) ---
  // Внимание: SGP30 и VL53L0X могут быть на разных каналах мультиплексора
  tcaselect(6); // Используем другой канал для VL53L0X
  if (vl53.begin()) {
    Serial.println("[OK] MGS-D20 (VL53L0X) найден");
  } else {
    Serial.println("[FAIL] MGS-D20 не найден");
  }

  // --- MGS-THP80 (BME280) на канале 0 ---
  tcaselect(0);
  if (bme280.begin(ADDR_BME280)) {
    Serial.println("[OK] MGS-THP80 (BME280) найден");
  } else {
    Serial.println("[FAIL] MGS-THP80 не найден");
  }

  // --- MGS-L75 (BH1750) на канале 0 ---
  tcaselect(0);
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("[OK] MGS-L75 (BH1750) найден");
  } else {
    Serial.println("[FAIL] MGS-L75 не найден");
  }

  // --- MGS-A6 (MPU6050) на канале 4 ---
  tcaselect(4);
  if (mpu.begin()) {
    Serial.println("[OK] MGS-A6 (MPU6050) найден");
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  } else {
    Serial.println("[FAIL] MGS-A6 не найден");
  }

  // --- MGS-WT1 (протечка) - цифровой пин, не через I2C ---
  pinMode(WT1_PIN, INPUT_PULLUP);
  Serial.println("[OK] MGS-WT1 (Water Leak) настроен");

  // --- MGS-SND504 (звук) - аналоговый пин ---
  pinMode(SND_PIN, INPUT);
  Serial.println("[OK] MGS-SND504 (Sound) настроен");

  // --- MGS-FR403 (пламя) - аналоговый пин ---
  pinMode(FLAME_PIN, INPUT);
  Serial.println("[OK] MGS-FR403 (Flame) настроен");

  bluetooth.println("Все датчики инициализированы");
  Serial.println("Готов к работе!");
}

// ========== Чтение данных с датчиков ==========
void readAllSensors() {
  String data = "{";
  
  // 1. MGS-THP80 (BME280) - температура, влажность, давление
  tcaselect(0);
  data += "\"THP80_temp\": " + String(bme280.readTemperature(), 1) + ",";
  data += "\"THP80_hum\": " + String(bme280.readHumidity(), 1) + ",";
  data += "\"THP80_press\": " + String(bme280.readPressure() / 133.322, 1) + ","; // в мм рт.ст.

  // 2. MGS-L75 (BH1750) - освещенность
  tcaselect(0);
  data += "\"L75_lux\": " + String(lightMeter.readLightLevel(), 1) + ",";

  // 3. MGS-FR403 - датчик пламени (аналоговое значение)
  data += "\"FR403_flame\": " + String(analogRead(FLAME_PIN)) + ",";

  // 4. MGS-SND504 - датчик звука
  data += "\"SND504_sound\": " + String(analogRead(SND_PIN)) + ",";

  // 5. MGS-WT1 - протечка воды
  data += "\"WT1_leak\": " + String(digitalRead(WT1_PIN) == LOW ? "\"WET\"" : "\"DRY\"") + ",";

  // 6. MGS-CLM60 (APDS-9960) - цвет и приближение
  tcaselect(3);
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
  tcaselect(4);
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
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
    data += "\"CO30_eco2\": 0,";
    data += "\"CO30_tvoc\": 0,";
  }

  // 9. MGS-D20 (VL53L0X) - расстояние
  tcaselect(6);
  VL53L0X_RangingMeasurementData_t measure;
  vl53.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    data += "\"D20_distance\": " + String(measure.RangeMilliMeter) + "";
  } else {
    data += "\"D20_distance\": -1"; // Вне диапазона
  }

  data += "}";
  
  // Отправляем JSON по Bluetooth
  bluetooth.println(data);
  
  // Дублируем в Serial Monitor для отладки
  Serial.println(data);
}

// ========== Основная программа ==========
void setup() {
  initSensors();
  delay(2000);
}

void loop() {
  readAllSensors();
  delay(5000); // Отправка данных каждые 5 секунд
}