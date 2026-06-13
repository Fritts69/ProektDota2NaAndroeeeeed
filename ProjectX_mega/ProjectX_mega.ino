#include <Wire.h>
#include <SPI.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// Pin definitions
#define DHT_PIN 2
#define DHT_TYPE DHT22
#define LM75A_ADDR 0x48

// Initialize sensors
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_BMP280 bmp280;

// Use hardware SPI for BMP280
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10

// Bluetooth baud rate
#define BT_BAUD 9600

unsigned long previousMillis = 0;
const long interval = 2000; // Send data every 2 seconds

struct SensorData {
  float lm75a_temp;
  float dht_temp;
  float dht_humidity;
  float bmp_temp;
  float bmp_pressure;
  bool lm75a_error;
  bool dht_error;
  bool bmp_error;
};

SensorData data;

void setup() {
  Serial.begin(BT_BAUD);
  Wire.begin();
  
  // Initialize DHT22
  dht.begin();
  delay(100);
  
  // Initialize BMP280 with SPI
  if (!bmp280.begin(BMP_CS)) {
    data.bmp_error = true;
  } else {
    bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,
                       Adafruit_BMP280::SAMPLING_X2,
                       Adafruit_BMP280::SAMPLING_X16,
                       Adafruit_BMP280::FILTER_X16,
                       Adafruit_BMP280::STANDBY_MS_500);
    data.bmp_error = false;
  }
  
  data.lm75a_error = false;
  data.dht_error = false;
  
  Serial.println("ARDUINO_START");
}

float readLM75A() {
  Wire.beginTransmission(LM75A_ADDR);
  Wire.write(0x00); // Temperature register
  if (Wire.endTransmission() != 0) {
    data.lm75a_error = true;
    return -999.0;
  }
  
  Wire.requestFrom(LM75A_ADDR, 2);
  if (Wire.available() >= 2) {
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    int16_t temp = (msb << 8) | lsb;
    temp >>= 7;
    data.lm75a_error = false;
    return temp * 0.5;
  }
  
  data.lm75a_error = true;
  return -999.0;
}

void readDHT22() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    data.dht_error = true;
    data.dht_humidity = -999.0;
    data.dht_temp = -999.0;
  } else {
    data.dht_error = false;
    data.dht_humidity = h;
    data.dht_temp = t;
  }
}

void readBMP280() {
  if (data.bmp_error) return;
  
  data.bmp_temp = bmp280.readTemperature();
  data.bmp_pressure = bmp280.readPressure() / 133.322; // Convert to mmHg
}

void sendData() {
  // Format: ARDUINO|LM75A_TEMP|DHT_TEMP|DHT_HUM|BMP_TEMP|BMP_PRESS|TIMESTAMP
  unsigned long timestamp = millis();
  
  String packet = "ARDUINO|";
  packet += String(data.lm75a_temp, 1) + "|";
  packet += String(data.dht_temp, 1) + "|";
  packet += String(data.dht_humidity, 1) + "|";
  packet += String(data.bmp_temp, 1) + "|";
  packet += String(data.bmp_pressure, 1) + "|";
  packet += String(timestamp);
  
  Serial.println(packet);
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Read all sensors
    data.lm75a_temp = readLM75A();
    readDHT22();
    readBMP280();
    
    // Send via Bluetooth
    sendData();
  }
}
