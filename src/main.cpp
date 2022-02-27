#include <Arduino.h>
#include <Wire.h> 
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 15 
#define ONE_WIRE_BUS 13

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
DHT dht(DHTPIN, DHTTYPE);

void setup()
{

  //start lcd
  lcd.init();
  lcd.backlight();
  lcd.clear();

  //Start Serial
  Serial.begin(9600);
  Serial.println("Start!");

  //start DHT
  dht.begin();

  //start ds18b20
  sensors.begin();
}

void read_18b20(void)
{
    sensors.requestTemperatures(); 
    delay(5000);
    float temperatureC = sensors.getTempCByIndex(0);

    Serial.print("ds18B20 sensor ");
    Serial.print(temperatureC);
    Serial.println("ºC");

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("[18B20 sensor]");
    lcd.setCursor(0,1);
    lcd.print("Temp:");
    lcd.print(temperatureC);
    lcd.print("C");
}

void read_dht(void)
{
   // Wait a few seconds between measurements.
  delay(2000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }


  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.printf("Read DHT sensor:\n");
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(F(" Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C \n"));

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Tmp:");
  lcd.print(t);
  lcd.print("C [DHT11]");
  lcd.setCursor(0,1);
  lcd.print("Hum:");
  lcd.print(h);
  lcd.print(" %HR");
}



void loop()
{
read_dht();
read_18b20();
}

