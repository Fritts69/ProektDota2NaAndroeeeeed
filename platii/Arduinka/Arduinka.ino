#include <Temperature_LM75_Derived.h> 
#include <DHT22.h> 
#include <Adafruit_BMP280.h>

#define BMP_SCK  (9) //scl
#define BMP_MISO (12) //sdo
#define BMP_MOSI (10) //sda
#define BMP_CS   (11) //csb

Generic_LM75 temperature;
DHT22 dht22(5);
Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

void setup() {
  while(!Serial) delay(1000);
  Serial.begin(38400);
  Serial1.begin(38400);
  Wire.begin();
  bmp.begin();
} 
 
void loop() {
  String s_tmp = String(temperature.readTemperatureC(), 1);
  String s_hmd = String(dht22.getHumidity(), 1);
  String s_prs = String(bmp.readPressure()*760/101325, 1);

  String data = "{'temp': " + s_tmp + ", 'humid': " + s_hmd + ", 'press': " + s_prs + "}";

  Serial.println(data);
  Serial1.println(data);
  delay(1000);

}
