#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
//#include <esp8266mDNS.h>

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266WebServer.h>

//#include <ESPAsyncTCP.h>
//#include <ESPAsyncWebServer.h>

#include <Arduino_JSON.h>

#include <ArduinoOTA.h>

#include <TaskScheduler.h>

#include <SPI.h>
#include <Wire.h>

#include "Adafruit_HT1632.h"

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Arduino_JSON.h>

#include <Adafruit_NeoPixel.h>

String httpGETRequest(const char*);

String jsonBuffer;
String city = "Wake Forest";
String countryCode = "US";

String LAT="";
String LON="";
String openWeatherMapApiKey = "af97131a6dddc89d7e21bd1c05ff0de1";
//String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=imperial";
String serverPath = "http://api.openweathermap.org/data/2.5/weather?lat=35.972440&lon=-78.471870&APPID=af97131a6dddc89d7e21bd1c05ff0de1&units=imperial";

String Local_URLs[4]={"http://esp_th_a.lan/temp", "http://esp_th_g.lan/temp","http://esp_th_u.lan/temp","http://esp_th_d.lan/temp"};


const char *ssid = "TRNNET-2G";
const char *password = "ripcord1";

const char *hostname="ESP_CLOCK";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"pool.ntp.org",-4*3600);

WiFiClient client;

ESP8266WebServer server(80);

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define OLED_RESET     A0 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(128, 32, &Wire, -1 );
Adafruit_HT1632LEDMatrix matrix = Adafruit_HT1632LEDMatrix(D6,D3,D4);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, D5, NEO_GRB + NEO_KHZ800);

JSONVar myObject;

void updateDisplay(void);
void updateWeather1(void);
void updateWeather(void);
void updateLocal(void);

Scheduler runner;
Task t1(1000, TASK_FOREVER, &updateDisplay);
Task t2(60000, TASK_FOREVER,&updateWeather);
Task t3(60000,TASK_FOREVER,&updateLocal);

String deg2dir(int deg){
   String dir;

   if ((deg>345 && deg<=360) || (deg>=0 && deg <=15))
      dir="N";
   else if (deg>15 && deg<=45)
      dir="NE";
   else if (deg>45 && deg<=75)
      dir="ENE";
   else if (deg>75 && deg<=105)
      dir="E";
   else if (deg>105 && deg<=135)
      dir="ESE";
   else if (deg>135 && deg<=165)
      dir="SE";
   else if (deg>165 && deg<=195)
      dir="S";
   else if (deg>195 && deg<=225)
      dir="SW";
   else if (deg>225 && deg <=255)
      dir="WSW";
   else if (deg>255 && deg<=285)
      dir="W";
   else if (deg>285 && deg<=315)
      dir="WNW";
   else if (deg>315 && deg<=345)
      dir="NW";

   return(dir);
}

void getbasic(){

   String message;
   
   message+=(double) myObject["main"]["temp"];
   message+="\t";
   message+=(double) myObject["main"]["humidity"];
   message+="\t";
   message+=(int) myObject["main"]["pressure"];
   message+="\n";

    server.send(200, "text/plain", message);

}

void getjson(){
   server.send(200, "text/plain", jsonBuffer);
}

void updateLocal(){
   String buf;
   String local_temps[4];
   int start,stop;


   for (int i=0;i<=3;i++){
      buf=httpGETRequest(Local_URLs[i].c_str());
      Serial.println(buf);
      start=buf.indexOf("\t");
      Serial.println(start);
      stop=buf.indexOf("\t",start+1);
      Serial.println(stop);
      local_temps[i]=buf.substring(start+1,stop-1);
   }

   display.clearDisplay();
   display.setTextSize(2,2);             
   display.setTextColor(SSD1306_WHITE);        
   display.setCursor(0,0);             

   for (int i=0;i<=3;i++){
      display.print(local_temps[i]);
      display.print(" ");
   }
   display.display();
}

void updateDisplay(){

    unsigned min,hour;
    static unsigned index=0;

    min=timeClient.getMinutes();
    hour=timeClient.getHours();
    if (hour>12)
       hour=hour-12;

/*
    matrix.setRotation(2);
    matrix.setCursor(0,0);
    matrix.clearScreen();
  
    if(hour<10)
       matrix.print("0");   
    matrix.print(hour);
    if(min<10)
       matrix.print("0");
    matrix.print(min);

     matrix.setCursor(0,8);

     switch (index) {
       case 0: matrix.print(myObject["main"]["temp"]);
               break;
       case 1: matrix.print(myObject["main"]["humidity"]);
               break;
       case 2: //matrix.print((double) myObject["main"]["pressure"]*0.02953);
               matrix.print(myObject["main"]["pressure"]);
               break;
       case 3: matrix.print((int) myObject["main"]["pressure"]-hour_pressure);
               break;
     }

     */

    if (index==0){
    matrix.setRotation(2);
    matrix.setCursor(0,0);
    matrix.clearScreen();
    if(hour<10)
       matrix.print("0");   
    matrix.print(hour);
    if(min<10)
       matrix.print("0");
     matrix.print(min);
     matrix.setCursor(0,8);
     matrix.print(myObject["main"]["temp"]);
     matrix.writeScreen();
     matrix.setRotation(0);

     strip.setPixelColor(0,0,128,0);
     strip.show();

     } else if(index==15) {
      matrix.setRotation(2);
      matrix.setCursor(0,0);
      matrix.clearScreen();
      matrix.print(myObject["main"]["pressure"]);
      matrix.setCursor(0,8);
      matrix.print(myObject["main"]["humidity"]);
      matrix.print("%");
      matrix.writeScreen();
      matrix.setRotation(0);

      strip.setPixelColor(0,104,102,0);
      strip.show();

     } else if (index==17) {
      matrix.setRotation(2);
      matrix.setCursor(0,0);
      matrix.clearScreen();
      matrix.print(deg2dir(myObject["wind"]["deg"]));
      matrix.setCursor(0,8);
      if ((int) myObject["wind"]["speed"]<10)
         matrix.print(0);
      matrix.print((int) myObject["wind"]["speed"]);
      if ((int) myObject["wind"]["gust"]<10)
         matrix.print(0);
      matrix.print((int) myObject["wind"]["gust"]);
      matrix.writeScreen();
      matrix.setRotation(0);

      strip.setPixelColor(0,0,0,128);
      strip.show();

     } else if (index==19){
        if (myObject["precipitation"]["mode"]!=null){
          matrix.setRotation(2);
          matrix.setCursor(0,0);
          matrix.clearScreen();
          matrix.print(myObject["precipitation"]["mode"]);
          matrix.setCursor(0,8);
          matrix.print(myObject["precipitation"]["value"]);
          matrix.print("%");
          matrix.writeScreen();
          matrix.setRotation(0);
        }else 
           index=-1;
     } else if (index==21){
         index=-1;
     }     
     index=index+1;

}

void updateWeather(){

   String buf;

   jsonBuffer = httpGETRequest(serverPath.c_str());
   myObject = JSON.parse(jsonBuffer);

}

void connect() {
  WiFi.mode(WIFI_STA);

  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("attempting connect to AP");
    delay(2000);
  }
}

void setup(void) {

    Serial.begin(9600);
    connect();
    delay(5000);
    ArduinoOTA.begin();

    timeClient.begin();
    timeClient.update();

    server.on(F("/basic"),HTTP_GET,getbasic);
    server.on(F("/getjson"),HTTP_GET,getjson);
    server.begin();
    Serial.println("HTTP server started");

    matrix.begin(ADA_HT1632_COMMON_16NMOS);
    matrix.clearScreen();

    strip.begin();

    runner.init();
    runner.addTask(t1);
    runner.addTask(t2);
    runner.addTask(t3);

   jsonBuffer = httpGETRequest(serverPath.c_str());
   myObject = JSON.parse(jsonBuffer);
    t1.enable();
    t2.enable();
    t3.enable();

    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

    strip.setPixelColor(0,104,102,0);
    strip.show();

}

void loop(void) {


   ArduinoOTA.handle();
   runner.execute();
   timeClient.update();

  server.handleClient();
}

String httpGETRequest(const char* serverName) {
  HTTPClient http;
  
  http.setTimeout(20000);
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();

  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  //Serial.println(payload);
  // Free resources
  http.end();

  return payload;
}