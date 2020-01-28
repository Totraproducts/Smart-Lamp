 /********************************************************
  *            Program for IOT Samrt Lamp              *
  *                                                    *
  * Author:   Rahul Sharma                             *
  *                                                    *
  * Purpose:  Demonstration of a simple program.       *
  *                                                    *
  * Usage:    Upload it to esp8266                     *
********************************************************/

/***************** INCLUDES *******************/

#include <Arduino.h>
#include "Adafruit_APDS9960.h"
#include <ESP8266WebServer.h>
#include <AutoConnect.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <esp8266httpclient.h>
#include <Wire.h>
#include <ThingSpeak.h>
#include <WiFiUdp.h>
#include <U8g2lib.h>
#include <NTPClient.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //  https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h> // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <StreamString.h>

/***************** MACROS *******************/

#define BLYNK_PRINT Serial
#define red D8
#define green D7
#define blue D6

#define pingPin 3  //Rx
#define echoPin 1  //Tx

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MyApiKey "538d6c48-dd8e-4a62-8a77-85ac9e56ebf2" // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define HEARTBEAT_INTERVAL 300000 // 5 Minutes

ESP8266WebServer  Server;
AutoConnect       Portal(Server);
WiFiClient        client;
WidgetBridge      bridge1(V1);              // Bridge widget on virtual pin 1
BlynkTimer        timer;                    // Timer for blynking
AutoConnectConfig acConfig;
ESP8266WiFiMulti  WiFiMulti;
WebSocketsClient  webSocket;

// Define NTP Client to get time
WiFiUDP           ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800,60000);

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);

/***************** GLOBAL VARIABLES *******************/

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).

char auth[]            = "E5ow8NqRrM5i1uO-fIdPw--ViQDnrfze";

// Your WiFi credentials.
char ssid[]                  = "idk";
char pass[]                  = "idk";
int rcount                   = 0;
int gcount                   = 0;
int bcount                   = 0;
int redbutton                = 0;
int greenbutton              = 0;
int bluebutton               = 0;
int pinValue                 = 0;
int pinValue_randomColor     = 0;
unsigned long time_now       = 0;
unsigned long time_clock      = 0;
const int FieldLamp1         = 1;
const int FieldLamp2         = 2;
bool lamp2TriggerVal         = false;
int colorIdx                 = 0;
int handCount                = 0;
int handCount2               = 0;
bool gesturePause            = false;
uint64_t heartbeatTimestamp  = 0;
bool isConnected             = false;

/*Virtual Ports

        V0 = To set color according to weather
        V1 = To set random Color
        V2 = To send Red color to Lamp-2
        V3 = To send Green color to Lamp-2
        V4 = To send Blue color to Lamp-2

*/

Adafruit_APDS9960 apds;

/***************** GPIO PIN DEFINES *******************/

const int rButton = 14;   //GPIO2 / TxD1
const int gButton = 0;    //GPIO0 / D3
const int bButton = 16;    //GPIO14 / SLCK

/***************** RANDOM COLOR ARRAY *******************/
unsigned char random_color_array[200][3] =  {{53, 214, 82}, {150, 44, 213}, {9, 191, 38}, {33, 8, 25}, {151, 13, 19}, {27, 209, 176},
                                             {135, 119, 36}, {72, 117, 208}, {151, 82, 194}, {31, 89, 247}, {96, 143, 147}, {65, 75, 83},
                                             {141, 253, 103}, {41, 27, 183}, {121, 4, 167}, {21, 201, 6}, {25, 230, 89}, {124, 13, 175},
                                             {55, 164, 226}, {116, 106, 162}, {61, 253, 108}, {156, 173, 221}, {9, 219, 63}, {127, 224, 224},
                                             {142, 60, 85}, {60, 240, 56}, {91, 220, 162}, {50, 242, 96}, {134, 161, 153}, {153, 238, 54},
                                             {68, 186, 6}, {128, 79, 70}, {156, 27, 96}, {42, 136, 122}, {50, 220, 65}, {69, 29, 16}, {71, 33, 254},
                                             {139, 250, 105}, {64, 117, 233}, {87, 33, 221}, {76, 18, 147}, {12, 6, 153}, {156, 144, 34},
                                             {87, 236, 83}, {116, 114, 248}, {132, 125, 185}, {158, 88, 143}, {25, 111, 168}, {150, 173, 224},
                                             {4, 232, 93}, {156, 196, 201}, {147, 135, 100}, {150, 56, 67}, {160, 245, 60}, {44, 9, 48}, {93, 82, 52},
                                             {117, 148, 93}, {59, 182, 153}, {51, 193, 55}, {41, 123, 199}, {103, 43, 36}, {6, 251, 64}, {78, 25, 77},
                                             {137, 205, 69}, {89, 181, 242}, {101, 206, 61}, {82, 166, 228}, {18, 118, 153}, {9, 131, 15}, {126, 79, 2},
                                             {55, 48, 134}, {3, 238, 15}, {137, 86, 133}, {17, 142, 191}, {122, 29, 118}, {113, 86, 59}, {148, 126, 214},
                                             {92, 95, 220}, {41, 111, 160}, {11, 107, 50}, {40, 43, 24}, {140, 198, 28}, {54, 231, 182}, {142, 82, 80},
                                             {19, 44, 239}, {136, 160, 101}, {10, 220, 214}, {130, 172, 205}, {45, 104, 115}, {8, 1, 117}, {37, 174, 32},
                                             {44, 157, 96}, {129, 243, 202}, {6, 255, 133}, {26, 155, 57}, {124, 135, 138}, {109, 75, 140}, {119, 74, 232},
                                             {14, 210, 97}, {159, 115, 244}, {147, 237, 98}, {25, 219, 102}, {92, 130, 178}, {11, 3, 159}, {58, 95, 69},
                                             {115, 114, 159}, {110, 6, 142}, {105, 50, 122}, {106, 119, 198}, {4, 102, 202}, {60, 169, 182}, {73, 138, 145},
                                             {160, 133, 28}, {148, 196, 7}, {119, 226, 105}, {47, 11, 107}, {108, 48, 9}, {118, 43, 22}, {113, 236, 116},
                                             {83, 211, 219}, {100, 59, 34}, {36, 235, 133}, {126, 240, 70}, {135, 164, 205}, {46, 246, 53}, {142, 141, 232},
                                             {117, 18, 53}, {83, 147, 3}, {42, 78, 211}, {66, 167, 163}, {78, 204, 26}, {149, 138, 130}, {40, 216, 20},
                                             {62, 114, 235}, {81, 47, 172}, {154, 182, 153}, {132, 186, 232}, {150, 91, 50}, {92, 79, 23}, {17, 115, 232},
                                             {56, 176, 53}, {38, 249, 212}, {110, 128, 42}, {57, 216, 151}, {11, 107, 124}, {79, 60, 156}, {109, 130, 67},
                                             {52, 177, 229}, {0, 60, 77}, {2, 64, 59}, {57, 184, 149}, {125, 89, 148}, {14, 165, 56}, {3, 157, 194},
                                             {81, 198, 159}, {126, 111, 94}, {57, 3, 155}, {71, 246, 136}, {118, 205, 241}, {128, 227, 149}, {121, 250, 77},
                                             {27, 10, 97}, {147, 86, 177}, {45, 182, 247}, {23, 247, 214}, {80, 66, 95}, {115, 100, 55}, {62, 144, 43},
                                             {42, 129, 141}, {68, 12, 114}, {46, 121, 89}, {53, 241, 1}, {46, 162, 48}, {79, 98, 170}, {49, 201, 103},
                                             {157, 66, 85}, {61, 37, 254}, {127, 244, 120}, {42, 155, 50}, {63, 6, 200}, {110, 199, 66}, {112, 216, 58},
                                             {76, 105, 198}, {150, 152, 72}, {115, 193, 182}, {152, 211, 238}, {116, 198, 147}, {4, 53, 51}, {138, 161, 202},
                                             {102, 211, 138}, {124, 92, 32}, {126, 86, 242}, {33, 123, 101}, {90, 180, 218}, {117, 9, 101}, {139, 94, 92},
                                             {104, 35, 84}, {19, 8, 40}, {112, 161, 102}, {135, 63, 109}};

/***************** STRUCTURE *******************/
struct rgbvalue{
    int r,g,b;
};

typedef struct rgbvalue RGBvalueObj;

/***************** FUNCTIONS *******************/

/**********************************************************
 * Function Name: rootPage
 * Functionality: Show name on front page of wifi connect
 * Notes        : Call at setup
***********************************************************/
void rootPage()
{
  char content[] = "TOTRA PRODUCTS";
  Server.send(200, "text/plain", content);
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
    colorIdx = random(0, 200);
    analogWrite(red,   (int)random_color_array[colorIdx][0]);
    analogWrite(green, (int)random_color_array[colorIdx][1]);
    analogWrite(blue,  (int)random_color_array[colorIdx][2]);
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

    http.begin("http://api.openweathermap.org/data/2.5/weather?q=New%20Delhi&APPID=d634395ad4513eafb0d9507bc2ba5eb8");  //Specify request destination
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
 * Function Name: RetrieveTSChannelData
 * Functionality: Receive Thingspeak channel Data
 * Notes        : Control hand up down gesture
***********************************************************/
/*void RetrieveTSChannelData()   // Receive data from Thingspeak
{
  int statusCode;
  long field2Data = ThingSpeak.readLongField(Channelno, FieldLamp2, APIreadkey);
  statusCode = ThingSpeak.getLastReadStatus();
  if (statusCode == 200)
  {
    Serial.println(field2Data);
  }
}*/

/**********************************************************
 * Function Name: setLamp2Color
 * Functionality: Set Lamp-2 color
 * Notes        : Call with Timer iterrupt
***********************************************************/
void setLamp2Color() // Here we will send HIGH or LOW once per 5 second
{
  // Send value to another device
  if (lamp2TriggerVal)
  {
    int red_loc, green_loc, blue_loc;

    red_loc   = pulseIn(red, HIGH);
    green_loc = pulseIn(green, HIGH);
    blue_loc  = pulseIn(blue, HIGH);

    bridge1.virtualWrite(V2, red_loc); // Sends 1 value to BLYNK_WRITE(V2) handler on receiving side.
    bridge1.virtualWrite(V3, green_loc);
    bridge1.virtualWrite(V4, blue_loc);
    //Serial.println("setLamp2Color...");
    lamp2TriggerVal = false;
    gesturePause    = false;
  }
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
 * Function Name: thingspeakUpdateForOtherLamp
 * Functionality: Detect gesture
 * Notes        :
***********************************************************/
void gestureDetect()
{
    int red_loc, green_loc, blue_loc;

    red_loc   = pulseIn(red, HIGH);
    green_loc = pulseIn(green, HIGH);
    blue_loc  = pulseIn(blue, HIGH);

    analogWrite(red,0);
    analogWrite(green,0);
    analogWrite(blue,0);
    delay(300);
    analogWrite(red,160);
    analogWrite(green,255);
    analogWrite(blue,255);
    delay(300);
    analogWrite(red,0);
    analogWrite(green,0);
    analogWrite(blue,0);
    delay(300);
    analogWrite(red,  red_loc);
    analogWrite(green,green_loc);
    analogWrite(blue, blue_loc);

    lamp2TriggerVal = true;
    gesturePause    = true;
}

/**********************************************************
 * Function Name: handleGesture
 * Functionality: hand gesture prediction fuction

 * Notes        :
***********************************************************/
void handleGesture()
{
  //Serial.println("In Gesture Sensor function!");
  uint8_t gesture = apds.readGesture();
  //Serial.println("Function done!");
  if(gesture == APDS9960_DOWN)
  {
    //Serial.println("v");
  }
  if(gesture == APDS9960_UP)
  {
    //Serial.println("^");
    gestureDetect();
  }
  if(gesture == APDS9960_LEFT)
  {
    //Serial.println("<");
    colorIdx -= 1;
    if(colorIdx < 0)
    {
        colorIdx = 199;
    }
    analogWrite(red,   (int)random_color_array[colorIdx][0]);
    analogWrite(green, (int)random_color_array[colorIdx][1]);
    analogWrite(blue,  (int)random_color_array[colorIdx][2]);

  }
  if(gesture == APDS9960_RIGHT)
  {
    //Serial.println(">");
      colorIdx += 1;
      if(colorIdx > 199)
      {
        colorIdx = 0;
    }
    analogWrite(red,   (int)random_color_array[colorIdx][0]);
    analogWrite(green, (int)random_color_array[colorIdx][1]);
    analogWrite(blue,  (int)random_color_array[colorIdx][2]);
  }
}

/**********************************************************
 * Function Name: microsecondsToCentimeters
 * Functionality: convert time to distance(cm)
 * Notes        :
***********************************************************/
long microsecondsToCentimeters(long microseconds) {
   return microseconds / 29 / 2;
}

/**********************************************************
 * Function Name: handleGestureUS
 * Functionality: hand gesture prediction fuction with ultrasonic
 * Notes        :
***********************************************************/
void handleGestureUS()
{
   long duration, cm;
   pinMode(pingPin, OUTPUT);
   digitalWrite(pingPin, LOW);
   delayMicroseconds(2);
   digitalWrite(pingPin, HIGH);
   delayMicroseconds(10);
   digitalWrite(pingPin, LOW);
   pinMode(echoPin, INPUT);
   duration = pulseIn(echoPin, HIGH);
   cm = microsecondsToCentimeters(duration);
   if(cm > 40) cm = -1;
   if((cm >= 1)&&(cm <= 10)&&(!gesturePause))
   {
    //Serial.println(handCount);
      handCount ++;
      if(handCount>=200)
      {
        //Serial.println("Set other lamp...");
        gestureDetect();
        handCount = 0;
      }
   }
   else
   {
      handCount = 0;
   }
   if(cm > 10)
   {
      handCount2++;
      if(handCount2 >= 200)
      {
         analogWrite(red,   random(0, 460));
         analogWrite(green, random(0, 1024));
         analogWrite(blue,  random(0, 1024));
         delay(1000);
      }
   }
   if(cm == -1)
   {
      handCount2 = 0;
   }
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
    analogWrite(red,   random(0, 460));
    analogWrite(green, random(0, 1024));
    analogWrite(blue,  random(0, 1024));
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
     analogWrite(red,   0);
     analogWrite(green, 0);
     analogWrite(blue,  0);
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
      //Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
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
        String action = json ["action"];

        if(action == "action.devices.commands.OnOff") { // Switch
            String value = json ["value"]["on"];
            //Serial.println(value);

            if(value == "true") {
                googleTurnOn(deviceId);
            } else {
                googleTurnOff(deviceId);
            }
        }
        else if (action == "test") {
            //Serial.println("[WSc] received test command from sinric.com");
        }
      }
      break;
    case WStype_BIN:
      //Serial.printf("[WSc] get binary length: %u\n", length);
      break;
    default: break;
  }
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

/**********************************************************
 * Function Name: BLYNK_CONNECTED
 * Functionality: stablish bridge connection
 * Notes        :
***********************************************************/
BLYNK_CONNECTED() {
  bridge1.setAuthToken("jHXXwo1UFze-idCneKHAEKmF2T_H4RC2"); // Place the AuthToken of Lamp-2
}

/**********************************************************
 * Function Name: setup
 * Functionality: Arduino setup function
 * Notes        :
***********************************************************/
void setup()
{
  //Serial.begin(9600);
  Server.on("/", rootPage);
  acConfig.psk = "totra";
  Portal.config(acConfig);
  if (Portal.begin()) {
    //Serial.println("HTTP server:" + WiFi.localIP().toString());
  }
  Blynk.begin(auth, ssid, pass);
  while (Blynk.connect() == false){}
  analogWrite(red,rcount);
  analogWrite(green,gcount);
  analogWrite(blue,bcount);

  pinMode(rButton, INPUT);
  pinMode(gButton, INPUT);
  pinMode(bButton, INPUT);

  timeClient.begin();
  u8g2.begin();
  dispLogo();
  /*
  if(!apds.begin()){
    Serial.println("failed to initialize device! Please check your wiring.");
  }
  else Serial.println("Gesture initialized!");
  */
  //apds.enableGesture(true);
  //ThingSpeak.begin(client);
  colorIdx = random(0, 200);
  timer.setInterval(5000L, setLamp2Color);

  // server address, port and URL
  webSocket.begin("iot.sinric.com", 80, "/"); //"iot.sinric.com", 80

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);

  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets

  delay(10000);
  u8g2.clearBuffer();
  dispTimeTemp();
}

/**********************************************************
 * Function Name: loop
 * Functionality: Arduino loop function
 * Notes        :
***********************************************************/
void loop()
{
  redbutton = digitalRead(rButton);
  greenbutton = digitalRead(gButton);
  bluebutton = digitalRead(bButton);
  Blynk.run();
  timer.run();
  Portal.handleClient();
  /*if (redbutton == HIGH) // Read Red touch sensor
  {
     rcount+=40;
     if (rcount > 1023)
     {
        rcount = 0;
     }
     analogWrite(red,rcount);
     Serial.print("Red Value: ");
     Serial.println(rcount);
     delay(300);
  }*/
  /*if (greenbutton == HIGH) // Read Green touch sensor
  {
     gcount+=40;
     if (gcount > 1023)
     {
        gcount = 0;
     }
     analogWrite(green,gcount);
     Serial.print("Green Value: ");
     Serial.println(gcount);
     delay(300);
  }*/
  if (bluebutton == HIGH) // Read Blue touch sensor
  {
     bcount+=40;
     if (bcount > 1023)
     {
        bcount = 0;
     }
     analogWrite(blue,bcount);
     //Serial.print("Blue Value: ");
     //Serial.println(bcount);
     delay(300);
  }
  if ((pinValue) && (millis()>time_now+900000))
  {
    time_now=millis();
    mapColor(0,50,(int)getweather());
  }

  if(colorGlowState)
  {
    handleGestureUS();
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
