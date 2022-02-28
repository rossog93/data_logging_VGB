#include <Arduino.h>
#include <Wire.h> 
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 15 
#define ONE_WIRE_BUS 13
#define AnalogInput 12


// Replace with your network credentials
const char* ssid = "Zhone_6383";
const char* password = "znid306054787";
//const char* ssid = "Rosso_wireless_JP";
//const char* password = "mechi431799";


AsyncWebServer server(80);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
DHT dht(DHTPIN, DHTTYPE);

void initWiFi() {
  uint32_t notConnectedCounter = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");
  lcd.setCursor(0,1);
  lcd.print("Status:");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    lcd.print(".");
    delay(1000);

    notConnectedCounter++;
    if(notConnectedCounter > 15) { // Reset board if not connected after 5s
        Serial.println("Resetting due to Wifi not connecting...");
        ESP.restart();
    }

  }
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("WIFI Connected!! ");
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());
  delay(10000);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(200, "text/plain", "Hi! I am ESP8266.");});
  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

} 


void Scanner ()
{
  Serial.println ();
  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;

  Wire.begin();
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission (i);          // Begin I2C transmission Address (i)
    if (Wire.endTransmission () == 0)  // Receive 0 = success (ACK response) 
    {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);     // PCF8574 7 bit address
      Serial.println (")");
      count++;
    }
  }
  Serial.print ("Found ");      
  Serial.print (count, DEC);        // numbers of devices
  Serial.println (" device(s).");
}


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

  //init WIFI
  initWiFi();

  //I2C scanner
  //Scanner();
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
    lcd.print("[ds18b20 sensor]");
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
  lcd.print("C [DHT]");
  lcd.setCursor(0,1);
  lcd.print("Hum:");
  lcd.print(h);
  lcd.print(" %HR");
}

void read_analog_input(void)
{
  float milivots;
  milivots = analogReadMilliVolts(AnalogInput);
  Serial.println("Analog read:");
  Serial.print(milivots);
  Serial.print("mV");
}

void loop()
{
read_dht();
read_18b20();
}

