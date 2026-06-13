#include <Wire.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>

MPU6050 mpu;  // MGS-A6 (гиро+аксель)
SoftwareSerial btMega(14, 15); // BT HC-05

#define CO30_PIN A0

void setup() {
  Serial.begin(9600);
  btMega.begin(9600);
  Wire.begin();
  mpu.initialize();
}

void loop() {
  // A6
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // CO30
  int co2_raw = analogRead(CO30_PIN);
  float co2_ppm = map(co2_raw, 0, 1023, 400, 5000); // калибровка

  // D20 (лазерный дальномер — через I2C)
  Wire.requestFrom(0x62, 2);
  int distance = Wire.read() << 8 | Wire.read();

  String data = String("AX:") + ax +
                ",AY:" + ay +
                ",AZ:" + az +
                ",CO2:" + co2_ppm +
                ",DIST:" + distance;

  btMega.println(data);
  delay(3000);
}
