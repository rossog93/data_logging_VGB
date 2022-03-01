#include <Arduino.h>
#include <Wire.h> 
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <Adafruit_BMP280.h>

#include <InfluxDbClient.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>


#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 15 
#define ONE_WIRE_BUS 13
#define AnalogInput 12

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  300        /* Time ESP32 will go to sleep (in seconds) */


#define INFLUXDB_URL "http://18.231.104.230:8086"
#define INFLUXDB_BUCKET "ESP32_TEST"
#define INFLUXDB_TOKEN "server token"
#define INFLUXDB_ORG "org id"
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point data("from_esp32");


// Replace with your network credentials
const char* ssid = "Zhone_6383";
const char* password = "znid306054787";
//const char* ssid = "Rosso_wireless_JP";
//const char* password = "mechi431799";


AsyncWebServer server(80);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
//BMP280 address:0x76
Adafruit_BMP280 bmp; // I2C

DHT dht(DHTPIN, DHTTYPE);

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}


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
    delay(1500);
    notConnectedCounter++;
    if(notConnectedCounter > 20) { // Reset board if not connected after 5s
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

  //Start bmp280
  bmp.begin(0x76);

  //I2C scanner
  Scanner();

  //ESP to sleep
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  //init WIFI
  initWiFi();


}



void push_data(void)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("InFlux Writing: ");
  client.pointToLineProtocol(data);
  lcd.setCursor(0,1);
  lcd.print("done!");


  if (!client.writePoint(data)) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("InfluxDB write failed: ");
    lcd.setCursor(0,1);
    lcd.print(client.getLastErrorMessage());
    delay(5000);}
}


void read_bmp280(void)
{
  
//Variables
float pressure;		//To store the barometric pressure (Pa)
float temperature;	//To store the temperature (oC)
int altimeter; 		//To store the altimeter (m) (you can also use it as a float variable)

	//Read values from the sensor:
	pressure = bmp.readPressure();
	temperature = bmp.readTemperature();
	altimeter = bmp.readAltitude (1000); //Change the "1050.35" to your city current barrometric pressure (https://www.wunderground.com)
	
  //Print values to serial monitor:
	Serial.print(F("Pressure: "));
  Serial.print(pressure);
  Serial.print(" Pa");
  Serial.print("\t");
  Serial.print(("Temp: "));
  Serial.print(temperature);
  Serial.print(" oC");
	Serial.print("\t");  
  Serial.print("Altimeter: ");
  Serial.print(altimeter); // this should be adjusted to your local forcase
  Serial.println(" m");
    
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("[bmp280]-T:");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("P: ");
  lcd.print(pressure);
  lcd.print(" Pa");
  delay(5000);


  data.clearFields();
  data.addField("bmp280-temp",temperature);
  data.addField("bmp280-pres",pressure);
  data.addField("Altimeter",altimeter);
  push_data();

}

void read_18b20(void)
{
    sensors.requestTemperatures(); 
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

    data.clearFields();
    data.addField("ds18b20",temperatureC);
    push_data();
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

  data.clearFields();
  data.addField("dht-temp",t);
  data.addField("dht-hum",h);

  push_data();

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
read_bmp280();

lcd.clear();
lcd.setCursor(0,0);
lcd.println("Going to sleep...");
lcd.setCursor(0,1);
lcd.print(String(TIME_TO_SLEEP) +" Seconds");
delay(5000);
lcd.noBacklight();
esp_deep_sleep_start();


}

