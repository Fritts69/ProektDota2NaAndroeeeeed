#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "SparkFun_APDS9960.h"
#include <SoftwareSerial.h>

// 1-Wire (THP80)
#define ONE_WIRE_PIN PB8
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

// I2C
SparkFun_APDS9960 apds9960;  // CLM60 (жесты, цвет, освещенность)
int wt1_address = 0x28;      // WT1 (датчик протечки)

// Bluetooth HC-05
SoftwareSerial btSerial(PA9, PA10);

// Analog
#define L75_PIN PA0
#define SND504_PIN PA1

// Digital flame sensor
#define FLAME_PIN PB9

// RGB LED
#define RGB_R PA6
#define RGB_G PA7
#define RGB_B PA8

void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);
  sensors.begin();
  Wire.begin();
  apds9960.init();
  apds9960.enableLightSensor(true);
  pinMode(FLAME_PIN, INPUT);
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
}

void loop() {
  // THP80
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  // WT1 (water leak)
  Wire.requestFrom(wt1_address, 1);
  byte leak = Wire.read();

  // CLM60
  int ambient_light = apds9960.readAmbientLight();

  // L75
  int l75 = analogRead(L75_PIN);

  // SND504 (звук)
  int sound = analogRead(SND504_PIN);

  // FR403 (пламя)
  int flame = digitalRead(FLAME_PIN);

  // Формат передачи
  String data = String("T:") + temp +
                ",L:" + ambient_light +
                ",L75:" + l75 +
                ",SND:" + sound +
                ",FLAME:" + String(flame) +
                ",LEAK:" + String(leak);

  btSerial.println(data);

  // Сигнал RGB (например, при протечке — красный)
  if (leak) {
    analogWrite(RGB_R, 255);
    analogWrite(RGB_G, 0);
    analogWrite(RGB_B, 0);
  } else {
    analogWrite(RGB_R, 0);
    analogWrite(RGB_G, 255);
    analogWrite(RGB_B, 0);
  }

  delay(3000);
}
