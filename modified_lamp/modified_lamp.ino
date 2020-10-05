#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h> 
#include <StreamString.h>
#include <WiFiClient.h>

// MACROS

#define red D6
#define green D7
#define blue D8

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MyApiKey "538d6c48-dd8e-4a62-8a77-85ac9e56ebf2" // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define HEARTBEAT_INTERVAL 300000 // 5 Minutes

// GLOBAL OBJECTS

// Define NTP Client to get time
WiFiUDP           ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800,60000);

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);

char auth[]            = "E5ow8NqRrM5i1uO-fIdPw--ViQDnrfze";
WidgetBridge      bridge1(V1);              // Bridge widget on virtual pin 1
BlynkTimer        timer;                    // Timer for blynking
WiFiClient        client;

// Your WiFi credentials.
char ssid[]                  = "idk";
char pass[]                  = "idk";

// VARIABLES
unsigned long time_now       = 0;
unsigned long time_clock     = 0;
int pinValue                 = 0;
int pinValue_randomColor     = 0;
String RxData;
int RxValueArr[10]           = {0};
int dataCount                = 0;
int strt                     = 0;

// STRUCTURES
struct rgbvalue{
    int r,g,b;
};

typedef struct rgbvalue RGBvalueObj;


// FUNCTIONS

/**********************************************************
 * Function Name: mapColor
 * Functionality: Set Color based on temperature
 * Notes        :
***********************************************************/
void mapColor(int min1, int max1, int value)
{
    RGBvalueObj rgb;
    float minimum, maximum, ratio;
    minimum = (float)min1;
    maximum = (float)max1;
    ratio = 2 * (value-minimum) / (maximum - minimum);
    rgb.b = (int)max(0, 1023*(1 - ratio));
    rgb.r = (int)max(0, 1023*(ratio - 1));
    rgb.g = 1023 - rgb.b - rgb.r;
    analogWrite(red, rgb.r);
    analogWrite(green, rgb.g);
    analogWrite(blue, rgb.b);
}

/**********************************************************
 * Function Name: BLYNK_WRITE
 * Functionality: Exchange any data between Blynk app and
                  your hardware for weather color
 * Notes        : Vn is virtual port
***********************************************************/
BLYNK_WRITE(V0)
{
  float temp;
  pinValue = param.asInt();
  //Serial.print("Weather Button value is: ");
  //Serial.println(pinValue);
  if(pinValue)
  {
    time_now=0;
    temp = (int)getweather();
    mapColor(0,50,temp);
  }
  else
  {
    analogWrite(red,0);
    analogWrite(green,0);
    analogWrite(blue,0);
  }
}

/**********************************************************
 * Function Name: BLYNK_WRITE
 * Functionality: Exchange any data between Blynk app and
                  your hardware for random color
 * Notes        : Vn is virtual port
***********************************************************/
BLYNK_WRITE(V1)
{
  float temp;
  pinValue_randomColor = param.asInt();
  //Serial.print("Weather Button value is: ");
  //Serial.println(pinValue_randomColor);
  if(pinValue_randomColor)
  {
    analogWrite(red,   (int)random(0, 1023));
    analogWrite(green, (int)random(0, 1023));
    analogWrite(blue,  (int)random(0, 1023));
  }
  else
  {
    analogWrite(red,0);
    analogWrite(green,0);
    analogWrite(blue,0);
  }
}

/**********************************************************
 * Function Name: getweather
 * Functionality: Return temperature in degree celsius
 * Notes        :
***********************************************************/
float getweather()
{
  float temp=0;
  if (WiFi.status() == WL_CONNECTED)
  {
    StaticJsonDocument<1024> doc;
    HTTPClient http;  //Declare an object of class HTTPClient

    http.begin(client, "http://api.openweathermap.org/data/2.5/weather?q=New%20Delhi&APPID=d634395ad4513eafb0d9507bc2ba5eb8");  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request
    if (httpCode > 0)
    { //Check the returning code
      String payload = http.getString();   //Get the request response payload
      //Serial.println(payload);
      //JsonObject& root = doc.parseObject(payload);
      auto error = deserializeJson(doc,payload);
      if (error)
      {
       // Serial.print(F("deserializeJson() failed with code "));
        //Serial.println(error.c_str());
      }
      temp = doc["main"]["temp"];
      temp = temp-273.15;
      //Serial.print("Temperature: ");
      //Serial.println(temp);
    }
    http.end();   //Close connection
  }
  //Serial.println(temp);
  return temp;
}

/**********************************************************
 * Function Name: colorGlowState
 * Functionality: Identify whether light is glow or not
 * Notes        :
***********************************************************/
bool colorGlowState()
{
   int red_loc, green_loc, blue_loc;
   red_loc   = pulseIn(red, HIGH);
   green_loc = pulseIn(green, HIGH);
   blue_loc  = pulseIn(blue, HIGH);
   if((red_loc==0)||(green_loc==0)||(blue_loc==0)) return true;
   else return false;
}


/**********************************************************
 * Function Name: dispLogo
 * Functionality: Display TOTRA on Oled Display
 * Notes        :
***********************************************************/
void dispLogo()
{
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.firstPage();
  do {
    u8g2.setCursor(16, 25);
    u8g2.print(F("TOTRA"));
    u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),u8g2.getDisplayHeight() );
  } while ( u8g2.nextPage() );
}

/**********************************************************
 * Function Name: dispTimeTemp
 * Functionality: Display Time and temperatute on OLED Display
 * Notes        :
***********************************************************/
void dispTimeTemp()
{
  char h_str[3];
  char m_str[3],s_temp[3],day[9];
  int  temp;
  timeClient.update();
  strcpy(m_str, u8x8_u8toa(timeClient.getMinutes(), 2));    /* convert m to a string with two digits */
  strcpy(h_str, u8x8_u8toa(timeClient.getHours(), 2));
  temp = (int)getweather();
  strcpy(s_temp, u8x8_u8toa(temp, 2));
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_logisoso22_tn);
    u8g2.drawStr(5,27,h_str);
    u8g2.drawStr(35,27,":");
    u8g2.drawStr(46,27,m_str);
    u8g2.setFont(u8g2_font_ncenB12_tr);
    u8g2.drawStr(86,27,s_temp);
    u8g2.setCursor(110, 27);
    u8g2.print(F("'C"));
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(86, 9);
    u8g2.print(F(" Rahul"));
  } while ( u8g2.nextPage() );
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    wifiManager.autoConnect("Totra Lamp");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

    
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

        
    // Start Blynk
    Blynk.begin(auth, ssid, pass);

    // Init RGB Pin
    pinMode(red, OUTPUT); // Red
    pinMode(green, OUTPUT); // Green
    pinMode(blue, OUTPUT);// Blue
    analogWrite(red,0);
    analogWrite(green, 0);
    analogWrite(blue,0);
    analogWriteFreq(20000);
    
    u8g2.begin();
    u8g2.clearBuffer();
    dispLogo();
}

void loop() {
    // put your main code here, to run repeatedly:
    Blynk.run();
    RxData = Serial.readString();
    dataCount = 0;
    strt = 0;
    if (RxData.length() > 0)
    {
      //Serial.print(RxData);
      for(int i=0; i < RxData.length(); i++)
      { 
         if (RxData.substring(i, i+1) == ",")
         {
            RxValueArr[dataCount] = RxData.substring(strt, i).toInt();
            strt = i+1;
            dataCount++;
         }
          RxValueArr[dataCount] = RxData.substring(i+1).toInt();
      }
      Serial.println(RxValueArr[0]);
      Serial.println(RxValueArr[1]);
      Serial.println(RxValueArr[2]);
      Serial.println(RxValueArr[3]);
      Serial.println(RxValueArr[4]);
      Serial.println(RxValueArr[5]);
      Serial.println(RxValueArr[6]);
      Serial.println(RxValueArr[7]);
      Serial.println(RxValueArr[8]);

      if (RxValueArr[0] == 1)               // Touch Button
      {
        analogWrite(red,  RxValueArr[4]);
        analogWrite(green,RxValueArr[5]);
        analogWrite(blue, RxValueArr[6]);
      }
      else if (RxValueArr[0] == 2)         // Gesture
      { 
        if (RxValueArr[7] == 0)                 // UP
        {
          Serial.println("UP GESTURE DETECTED");
          analogWrite(red,   (int)random(0, 1023));
          analogWrite(green, (int)random(0, 1023));
          analogWrite(blue,  (int)random(0, 1023));
        }
        else if (RxValueArr[7] == 1)            // DOWN
        {
          Serial.println("DOWN GESTURE DETECTED");
          analogWrite(red,   0);
          analogWrite(green, 0);
          analogWrite(blue,  0);
        }
        else if (RxValueArr[7] == 2)            // LEFT
        {
          Serial.println("LEFT GESTURE DETECTED");
          analogWrite(red,   (int)random(0, 1023));
          analogWrite(green, (int)random(0, 1023));
          analogWrite(blue,  (int)random(0, 1023));
        }
        else if (RxValueArr[7] == 3)            // RIGHT
        {
          Serial.println("RIGHT GESTURE DETECTED");
          analogWrite(red,   (int)random(0, 1023));
          analogWrite(green, (int)random(0, 1023));
          analogWrite(blue,  (int)random(0, 1023));
        }
      }
      RxData = "";
    }
    
    if (millis()>time_clock+60000)
    {
       time_clock=millis();
       dispTimeTemp();
    }
}
