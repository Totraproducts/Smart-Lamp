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
WebSocketsClient  webSocket;

// Your WiFi credentials.
char ssid[]                  = "idk";
char pass[]                  = "idk";

// VARIABLES
unsigned long time_now                 = 0;
unsigned long time_clock               = 0;
int           pinValue                 = 0;
int           pinValue_randomColor     = 0;
String        RxData;
int           RxValueArr[10]           = {0};
int           dataCount                = 0;
int           strt                     = 0;
bool          isConnected              = false;
uint64_t      heartbeatTimestamp       = 0;
bool          lampState                = false;
int           ghomeRed                 = 0;
int           ghomeGreen               = 0;
int           ghomeBlue                = 0;
bool          ghomeState               = false;
int           globalRed                = 0;
int           globalGreen              = 0;
int           globalBlue               = 0;

// STRUCTURES
struct rgbvalue{
    int r,g,b;
};

typedef struct rgbvalue RGBvalueObj;


// FUNCTIONS

/**********************************************************
 * Function Name: setColo
 * Functionality: Set color of lamp
 * Notes        :
***********************************************************/
void setColor(int r, int g, int b)
{
  analogWrite(red,  r);
  analogWrite(green,g);
  analogWrite(blue, b);
  globalRed   = r;
  globalGreen = g;
  globalBlue  = b;
}

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
    setColor(rgb.r, rgb.g, rgb.b);
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
    setColor(0, 0, 0);
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
    setColor((int)random(0, 1023), (int)random(0, 1023), (int)random(0, 1023));
  }
  else
  {
    setColor(0, 0, 0);
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
 * Function Name: dispGesture
 * Functionality: Display Gesture on Oled Display
 * Notes        :
***********************************************************/
void dispGesture(int val)
{
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.firstPage();
  do {
    if(val == 0)
    {
      u8g2.setCursor(16, 25);
      u8g2.print(F("    ON"));
    }
    else if(val == 1)
    {
      u8g2.setCursor(16, 25);
      u8g2.print(F("   OFF"));
    }
    else if(val == 3)
    {
      u8g2.drawLine(54, 14, 74, 14);
      u8g2.drawLine(54, 15, 74, 15);
      u8g2.drawLine(54, 16, 74, 16);
      
      u8g2.drawLine(64, 8,  75, 17);
      u8g2.drawLine(65, 7,  76, 16);
      u8g2.drawLine(66, 6,  77, 15);
  
      u8g2.drawLine(64, 22, 75, 15);
      u8g2.drawLine(65, 23, 76, 16);
      u8g2.drawLine(66, 24, 77, 17);
    }
    else if(val == 2)
    {
      u8g2.drawLine(54, 14, 74, 14);
      u8g2.drawLine(54, 15, 74, 15);
      u8g2.drawLine(54, 16, 74, 16);

      u8g2.drawLine(64, 6,  52, 15);
      u8g2.drawLine(65, 7,  53, 15);
      u8g2.drawLine(63, 5,  50, 15);
  
      u8g2.drawLine(64, 22, 52, 15);
      u8g2.drawLine(63, 23, 50, 15);
      u8g2.drawLine(65, 21, 53, 15);
    }
  } while ( u8g2.nextPage() );\
  delay(700);
}

/**********************************************************
 * Function Name: dispTouch
 * Functionality: Display RGB Touch value on Oled Display
 * Notes        :
***********************************************************/
void dispTouch(int r, int g, int b)
{
  char red[3];
  char green[3];
  char blue[3];
  strcpy(red,   u8x8_u8toa(r/100, 2));
  strcpy(green, u8x8_u8toa(g/100, 2));
  strcpy(blue,  u8x8_u8toa(b/100, 2));
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.firstPage();
  do {
    u8g2.setCursor(25, 9);
    u8g2.print(F("  Red    : "));
    u8g2.drawStr(85,9,red);
    u8g2.setCursor(25, 19);
    u8g2.print(F("  Green : "));
    u8g2.drawStr(85,19,green);
    u8g2.setCursor(25, 29);
    u8g2.print(F("  Blue   : "));
    u8g2.drawStr(85,29,blue);
  } while ( u8g2.nextPage() );
  delay(1000);
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
    if((int)h_str < 7)
    {
      u8g2.setContrast(1);
    }
    else
    {
      u8g2.setContrast(100);
    }
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

/**********************************************************
 * Function Name: googleTurnOn
 * Functionality: turn on lamp with google home
 * Notes        :
***********************************************************/
void googleTurnOn(String deviceId)
{
  if (deviceId == "5e2474200c04793a3a801f5b") // Device ID of first device
  {
    //Serial.print("Turn on device id: ");
    //Serial.println(deviceId);
    setColor((int)random(0, 1023), (int)random(0, 1023), (int)random(0, 1023));
  }
}

/**********************************************************
 * Function Name: googleTurnOff
 * Functionality: turn off lamp with google home
 * Notes        :
***********************************************************/
void googleTurnOff(String deviceId)
{
   if (deviceId == "5e2474200c04793a3a801f5b") // Device ID of first device
   {
     //Serial.print("Turn off Device ID: ");
     //Serial.println(deviceId);
     setColor(0, 0, 0);
   }
}

/**********************************************************
 * Function Name: webSocketEvent
 * Functionality: connect with https://sinric.com/
 * Notes        :
***********************************************************/
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;
      //Serial.println("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      //Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      //Serial.printf("Waiting for commands from sinric.com ...\n");
      }
      break;
    case WStype_TEXT: {
        //Serial.printf("[WSc] get text: %s\n", payload);
        // Example payloads

        // For Switch  types
        // {"deviceId":"xxx","action":"action.devices.commands.OnOff","value":{"on":true}} // https://developers.google.com/actions/smarthome/traits/onoff
        // {"deviceId":"xxx","action":"action.devices.commands.OnOff","value":{"on":false}}

#if ARDUINOJSON_VERSION_MAJOR == 5
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6
        DynamicJsonDocument json(1024);
        deserializeJson(json, (char*) payload);
#endif
        String deviceId = json ["deviceId"];
        String action   = json ["action"];
        String values   = json ["value"];

        //Serial.println(values);
        
        if(action == "action.devices.commands.OnOff") { // Switch
            String value1 = json ["value"]["on"];
            //Serial.println(value);
            if(value1 == "true") 
            {
              googleTurnOn(deviceId);
              ghomeState = true;
            } 
            else 
            {
              googleTurnOff(deviceId);
              ghomeState = false;
            }
        }
        
        else if((action== "action.devices.commands.ColorAbsolute") && (ghomeState))
        {
          int colorint = json ["value"]["color"]["spectrumRGB"];
          unsigned char googleRed   = (unsigned char) (colorint>>16 & 0x0000FF);
          unsigned char googleGreen = (unsigned char) (colorint>>8 & 0x0000FF);
          unsigned char googleBlue  = (unsigned char) (colorint & 0x0000FF);
              
          Serial.println(action);
          Serial.println(colorint);
          /*Serial.println(googleBlue);
          Serial.println(googleGreen);
          Serial.println(googleRed);*/
  
          ghomeRed    = map(googleRed,0,255,0,1023);
          ghomeGreen  = map(googleGreen,0,255,0,1023);
          ghomeBlue   = map(googleBlue,0,255,0,1023);
          setColor(ghomeRed, ghomeGreen, ghomeBlue);
        }
        
        else if((action== "action.devices.commands.BrightnessAbsolute") && (ghomeState))
        {
          int brightness = json ["value"]["brightness"];
          analogWrite(red,  (int)(globalRed*brightness/100));
          analogWrite(green,(int)(globalGreen*brightness/100));
          analogWrite(blue, (int)(globalBlue*brightness/100));
        }
      }
      break;
    case WStype_BIN:
      //Serial.printf("[WSc] get binary length: %u\n", length);
      break;
    default: break;
  }
}


void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);

    // OLED Begin
    u8g2.begin();
    u8g2.clearBuffer();
    dispLogo();
    
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
    // Turn Light off
    setColor(0, 0, 0);
    analogWriteFreq(20000);

    webSocket.begin("iot.sinric.com", 80, "/"); //"iot.sinric.com", 80
    // event handler
    webSocket.onEvent(webSocketEvent);
    webSocket.setAuthorization("apikey", MyApiKey);
    // try again every 5000ms if connection has failed
    webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets

    u8g2.clearBuffer();
    delay(10000);
    dispTimeTemp();
}

void loop() {
    // put your main code here, to run repeatedly:
    Blynk.run();
    RxData = Serial.readString();
    dataCount = 0;
    strt = 0;
    if (RxData.length() > 0)
    {
      Serial.print(RxData);
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
      /*Serial.println(RxValueArr[0]);
      Serial.println(RxValueArr[1]);
      Serial.println(RxValueArr[2]);
      Serial.println(RxValueArr[3]);
      Serial.println(RxValueArr[4]);
      Serial.println(RxValueArr[5]);
      Serial.println(RxValueArr[6]);
      Serial.println(RxValueArr[7]);
      Serial.println(RxValueArr[8]);*/

      if (RxValueArr[0] == 1)               // Touch Button
      {
        setColor(RxValueArr[4], RxValueArr[5], RxValueArr[6]);
        dispTouch(RxValueArr[4], RxValueArr[5], RxValueArr[6]);
        dispTimeTemp();
      }
      else if (RxValueArr[0] == 2)         // Gesture
      { 
        if (RxValueArr[7] == 0)                 // UP
        {
          Serial.println("UP GESTURE DETECTED");
          setColor((int)random(0, 1023), (int)random(0, 1023), (int)random(0, 1023));
          if (!lampState)
          {
            dispGesture(0);
            dispTimeTemp();
          }
          lampState = true;
        }
        else if (RxValueArr[7] == 1)            // DOWN
        {
          Serial.println("DOWN GESTURE DETECTED");
          setColor(0, 0, 0);
          if (lampState)
          {
            dispGesture(1);
            dispTimeTemp();
          }
          lampState = false;
        }
        else if (RxValueArr[7] == 2)            // LEFT
        {
          if(lampState)
          {
            Serial.println("LEFT GESTURE DETECTED");
            setColor(RxValueArr[4], RxValueArr[5], RxValueArr[6]);
            dispGesture(2);
            dispTimeTemp();
          }
        }
        else if (RxValueArr[7] == 3)            // RIGHT
        {
          if(lampState)
          {
            Serial.println("RIGHT GESTURE DETECTED");
            setColor(RxValueArr[4], RxValueArr[5], RxValueArr[6]);
            dispGesture(3);
            dispTimeTemp();
          }
        }
      }
      RxData = "";
    }

    //Google home Sync code
    webSocket.loop();
    if(isConnected)
    {
      uint64_t now = millis();
  
      // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night.
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL)
        {
         heartbeatTimestamp = now;
         webSocket.sendTXT("H");
      }
    }
    
    if (millis()>time_clock+60000)
    {
       time_clock=millis();
       dispTimeTemp();
    }
}
