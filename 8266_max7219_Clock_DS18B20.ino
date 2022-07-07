#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 0 // Пин подключения OneWire шины, 0 (D3)
OneWire oneWire(ONE_WIRE_BUS); // Подключаем бибилотеку OneWire
DallasTemperature sensors(&oneWire); // Подключаем бибилотеку DallasTemperature
DeviceAddress temperatureSensors[3]; // Размер массива определяем исходя из количества установленных датчиков
uint8_t deviceCount = 0;

int pinCS = D4; 
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
char time_value[20];

// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> D7  (Same Pin for WEMOS)
// CS             -> D4  (Same Pin for WEMOS)
// CLK            -> D5  (Same Pin for WEMOS)

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 70; // In milliseconds
int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels
int m;


String temp;
//char t;
//String t;
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX); // Выводим адрес датчика в HEX формате 
  }
}
void setup() {
  Serial.begin(9600);
  sensors.begin(); // Иницилизируем датчики
  deviceCount = sensors.getDeviceCount(); // Получаем количество обнаруженных датчиков

  for (uint8_t index = 0; index < deviceCount; index++)
  {
    sensors.getAddress(temperatureSensors[index], index);
  }
 
  //INSERT YOUR SSID AND PASSWORD HERE

  WiFi.begin("LOGIN","zPASSu");

  //CHANGE THE POOL WITH YOUR CITY. SEARCH AT https://www.ntppool.org/zone/@
  
  configTime(0 * 3600, 0, "it.pool.ntp.org", "time.nist.gov");
  
  setenv("TZ", "GMT-1BST",1);
  
  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 1);    // The first display is position upside down
  matrix.setRotation(1, 1);    // The first display is position upside down
  matrix.setRotation(2, 1);    // The first display is position upside down
  matrix.setRotation(3, 1);    // The first display is position upside down
  matrix.fillScreen(LOW);
  matrix.write();
  
  while ( WiFi.status() != WL_CONNECTED ) {
    matrix.drawChar(2,0, 'W', HIGH,LOW,1); // H
    matrix.drawChar(8,0, 'I', HIGH,LOW,1); // HH  
    matrix.drawChar(14,0,'-', HIGH,LOW,1); // HH:
    matrix.drawChar(20,0,'F', HIGH,LOW,1); // HH:M
    matrix.drawChar(26,0,'I', HIGH,LOW,1); // HH:MM
    matrix.write(); // Send bitmap to display
    delay(250);
    matrix.fillScreen(LOW);
    matrix.write();
    delay(2000);
  }
}
void display_message(String message)
{
   for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    //matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background off, reverse to invert the image
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait/2);
   
  }
}
void loop() {
  m = map(analogRead(0),0,1024,0,12);
  matrix.setIntensity(m);
  matrix.fillScreen(LOW);
  time_t now = time(nullptr);
  String time = String(ctime(&now));
  time.trim();
  //Serial.println(time);
  time.substring(11,19).toCharArray(time_value, 10); 
  matrix.drawChar(2,0, time_value[0], HIGH,LOW,1); // H
  matrix.drawChar(8,0, time_value[1], HIGH,LOW,1); // HH  
  matrix.drawChar(14,0,time_value[2], HIGH,LOW,1); // HH:
  matrix.drawChar(20,0,time_value[3], HIGH,LOW,1); // HH:M
  matrix.drawChar(26,0,time_value[4], HIGH,LOW,1); // HH:MM
  matrix.write(); // Send bitmap to display
       
  delay(10000);
      
  matrix.fillScreen(LOW);
  Serial.println();
  sensors.requestTemperatures();
  for (int i = 0; i < deviceCount; i++)
  {
   // printAddress(temperatureSensors[i]); // Выводим название датчика
    Serial.print(": ");
    Serial.println(sensors.getTempC(temperatureSensors[i])); // Выводим температуру с датчика
    temp = (String)sensors.getTempC(temperatureSensors[i]);
    //display_message(temp+"C ");
  }
//  t = sensors.getTempC(); // Выводим температуру с датчика

 // float temp = (String)sensors.getTempC();
   
    display_message("Temp: "+temp+" 'C");

if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://iss.moex.com/iss/statistics/engines/currency/markets/selt/rates.json?iss.meta=off")) 
    { 
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          display_message("Dollar: "+payload.substring(344,375));            
            }
             http.end();
        } else {
            USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
    delay(10000);    

if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/MTSS?iss.meta=off&iss.only=marketdata&marketdata.columns=LAST"))
    { 
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();              
               display_message("MTC: "+payload.substring(90,99));          
            }
             http.end();
        } else {
            USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
{
if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/DSKY?iss.meta=off&iss.only=marketdata&marketdata.columns=LAST"))
    { 
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
           display_message("Detskiy mir: "+payload.substring(90,99));    
            }
             http.end();
        } else {
            USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
    delay(1000);
}
 {
if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/SNGSP?iss.meta=off&iss.only=marketdata&marketdata.columns=LAST"))
    { 
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
                        display_message("SNG P: "+payload.substring(90,97));     
            }
             http.end();
        } else {
            USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
    delay(1000);
}
{
if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/RTKMP?iss.meta=off&iss.only=marketdata&marketdata.columns=LAST"))
    { 
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          display_message("Rostelecom P: "+payload.substring(90,97)); 
            }
             http.end();
        } else {
            USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
    delay(1000);
}

{
if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/GAZP?iss.meta=off&iss.only=marketdata&marketdata.columns=LAST"))
    { 
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          display_message("Gazprom: "+payload.substring(90,99));
            }
             http.end();
        } else {
            USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
    delay(1000);
}
{
if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/NKNCP?iss.meta=off&iss.only=marketdata&marketdata.columns=LAST"))
    { 
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          display_message("HKHX P: "+payload.substring(90,97));
            }
             http.end();
        } else {
            USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
    delay(1000);
{
if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/HYDR?iss.meta=off&iss.only=marketdata&marketdata.columns=LAST"))
    { 
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          display_message("RusHydro: "+payload.substring(90,99));
            }
             http.end();
        } else {
            USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
    delay(1000);
{
if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/LSNGP?iss.meta=off&iss.only=marketdata&marketdata.columns=LAST"))
    { 
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          display_message("LenEnergo P: "+payload.substring(90,99));
            }
             http.end();
        } else {
            USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
    delay(1000);

{
if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/NVTK?iss.meta=off&iss.only=marketdata&marketdata.columns=LAST"))
    { 
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          display_message("Novatek: "+payload.substring(90,99));         
            }
             http.end();
        } else {
            USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
    delay(1000);
{
if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://iss.moex.com/iss/engines/stock/markets/shares/boards/TQBR/securities/UWGN?iss.meta=off&iss.only=marketdata&marketdata.columns=LAST"))
    { 
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          display_message("OBK: "+payload.substring(90,98));         
            }
             http.end();
        } else {
            USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
    delay(1000);
}}}}}}}}}}}}}}}}}
