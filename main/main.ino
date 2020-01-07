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

#include "Adafruit_APDS9960.h"
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <AutoConnect.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <esp8266httpclient.h>
#include <Wire.h>
#include <ThingSpeak.h>


/***************** MACROS *******************/

#define BLYNK_PRINT Serial
#define red D8
#define green D7
#define blue D6

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

ESP8266WebServer Server;
AutoConnect      Portal(Server);
WiFiClient       client;

/***************** GLOBAL VARIABLES *******************/

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).

char auth[]            = "E5ow8NqRrM5i1uO-fIdPw--ViQDnrfze";

// Your WiFi credentials.
char ssid[]                = "idk";
char pass[]                = "idk";
int rcount                 = 0;
int gcount                 = 0;
int bcount                 = 0;
int redbutton              = 0;
int greenbutton            = 0;
int bluebutton             = 0;
int pinValue               = 0;
int pinValue_randomColor   = 0;
unsigned long time_now     = 0;
unsigned long time_now2    = 0;
unsigned long time_now3    = 0;
const int FieldLamp1       = 1;
const int FieldLamp2       = 2;

// defines variables for distance sensor
long duration;
int distance;
const char* host                 = "api.thingspeak.com"; // ThingSpeak address
String host_str                  = "https://api.thingspeak.com";
unsigned long Channelno          = 875199;             // Thingspeak Read Key, works only if a PUBLIC viewable channel
const char * APIreadkey          = "LNMGX4DFAJK0MY5B";   // Thingspeak Read Key, works only if a PUBLIC viewable channel
String APIreadkey_str            = "LNMGX4DFAJK0MY5B";
String Channelno_str             = "875199";
const int httpPort               = 80;
const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
int handcount                    = 0;
int colorIdx                     = 0;

Adafruit_APDS9960 apds;

/***************** GPIO PIN DEFINES *******************/

const int rButton = 14;   //GPIO2 / TxD1
const int gButton = 0;    //GPIO0 / D3
const int bButton = 16;    //GPIO14 / SLCK

/***************** RANDOM COLOR ARRAY *******************/
unsigned char random_color_array[200][3] =  {{115, 126, 97}, {18, 59, 251}, {188, 9, 201}, {89, 168, 174}, {227, 50, 156},
                                             {131, 108, 181}, {68, 222, 45}, {235, 4, 221}, {96, 86, 188}, {71, 208, 26},
                                             {3, 110, 176}, {87, 145, 182}, {61, 15, 53}, {90, 204, 219}, {109, 186, 99},
                                             {168, 124, 39}, {13, 200, 116}, {14, 215, 249}, {166, 39, 37}, {225, 207, 160},
                                             {210, 0, 202}, {75, 210, 89}, {88, 10, 76}, {248, 237, 186}, {203, 22, 150},
                                             {32, 208, 196}, {18, 80, 171}, {195, 183, 119}, {104, 2, 70}, {242, 216, 196},
                                             {140, 118, 53}, {147, 88, 55}, {36, 150, 244}, {207, 166, 121}, {103, 228, 199},
                                             {156, 28, 131}, {213, 15, 8}, {134, 95, 12}, {184, 136, 11}, {198, 85, 230},
                                             {65, 254, 2}, {67, 91, 239}, {163, 189, 31}, {197, 229, 132}, {254, 156, 83},
                                             {181, 141, 28}, {73, 44, 106}, {212, 220, 140}, {108, 101, 22}, {71, 74, 239},
                                             {38, 172, 73}, {227, 115, 112}, {250, 227, 36}, {31, 149, 40}, {117, 146, 42},
                                             {235, 78, 253}, {104, 139, 123}, {131, 18, 246}, {187, 208, 239}, {95, 72, 204},
                                             {75, 79, 171}, {105, 153, 92}, {137, 22, 50}, {199, 239, 199}, {20, 57, 121},
                                             {70, 250, 157}, {131, 61, 1}, {140, 79, 122}, {234, 97, 237}, {38, 110, 13},
                                             {114, 187, 78}, {218, 179, 235}, {208, 192, 186}, {146, 137, 40}, {157, 149, 44},
                                             {99, 76, 98}, {45, 114, 30}, {171, 101, 8}, {248, 116, 116}, {140, 126, 31},
                                             {207, 86, 12}, {199, 132, 25}, {134, 158, 169}, {114, 78, 227}, {154, 226, 189},
                                             {116, 61, 230}, {145, 6, 255}, {31, 223, 197}, {218, 131, 120}, {121, 148, 45},
                                             {149, 84, 39}, {91, 228, 103}, {164, 211, 111}, {128, 115, 164}, {4, 238, 186},
                                             {253, 68, 185}, {180, 219, 24}, {37, 195, 202}, {191, 185, 151}, {69, 221, 201},
                                             {187, 196, 233}, {196, 151, 165}, {184, 55, 227}, {166, 43, 57}, {23, 54, 136},
                                             {217, 81, 29}, {102, 231, 159}, {194, 55, 151}, {109, 53, 56}, {68, 151, 247},
                                             {220, 77, 73}, {52, 230, 38}, {228, 11, 0}, {131, 115, 128}, {172, 86, 115},
                                             {105, 82, 109}, {101, 141, 130}, {65, 72, 102}, {105, 29, 157}, {68, 225, 16},
                                             {212, 255, 177}, {194, 124, 106}, {75, 97, 244}, {118, 229, 204}, {156, 162, 44},
                                             {210, 178, 36}, {241, 199, 194}, {73, 51, 216}, {9, 2, 152}, {232, 235, 154},
                                             {224, 24, 46}, {185, 25, 189}, {22, 198, 179}, {81, 0, 18}, {68, 84, 176},
                                             {74, 33, 78}, {71, 14, 57}, {247, 98, 148}, {172, 49, 109}, {19, 78, 159},
                                             {90, 218, 140}, {149, 80, 114}, {175, 66, 1}, {93, 189, 1}, {23, 225, 90},
                                             {190, 43, 86}, {222, 250, 108}, {147, 18, 25}, {117, 157, 240}, {121, 54, 6},
                                             {181, 188, 152}, {241, 128, 133}, {160, 25, 181}, {90, 188, 110}, {181, 48, 88},
                                             {48, 188, 247}, {111, 11, 55}, {195, 125, 185}, {18, 122, 213}, {194, 26, 17},
                                             {250, 157, 88}, {245, 206, 67}, {226, 156, 109}, {78, 62, 206}, {79, 42, 140},
                                             {221, 98, 217}, {170, 79, 115}, {158, 71, 234}, {227, 50, 181}, {144, 13, 3},
                                             {20, 28, 179}, {240, 159, 177}, {33, 94, 152}, {126, 55, 85}, {42, 71, 214},
                                             {140, 100, 117}, {200, 225, 144}, {188, 2, 3}, {226, 245, 106}, {6, 244, 28},
                                             {250, 177, 201}, {134, 86, 221}, {215, 167, 53}, {233, 237, 63}, {211, 161, 43},
                                             {31, 176, 199}, {217, 175, 76}, {205, 74, 212}, {254, 67, 85}, {254, 226, 142},
                                             {95, 141, 101}, {2, 116, 108}, {226, 50, 188}, {61, 106, 212}, {148, 26, 108},
                                             {148, 52, 28}, {67, 35, 135}, {191, 150, 25}, {2, 115, 92}, {20, 0, 36}};

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
    rgb.b = (int)max(0, 255*(1 - ratio));
    rgb.r = (int)max(0, 255*(ratio - 1));
    rgb.g = 255 - rgb.b - rgb.r;
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
  Serial.print("Weather Button value is: ");
  Serial.println(pinValue);
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
  Serial.print("Weather Button value is: ");
  Serial.println(pinValue_randomColor);
  if(pinValue_randomColor)
  {

    analogWrite(red,   (int)random_color_array[colorIdx][0]);
    analogWrite(green, (int)random_color_array[colorIdx][1]);
    analogWrite(blue,  (int)random_color_array[colorIdx][2]);
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
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
      }
      temp = doc["main"]["temp"];
      temp = temp-273.15;
      Serial.print("Temperature: ");
      Serial.println(temp);
    }
    http.end();   //Close connection
  }
  Serial.println(temp);
  return temp;
}

/**********************************************************
 * Function Name: skipResponseHeaders
 * Functionality: Wait until there is some data and skip headers
 * Notes        :
***********************************************************/
bool skipResponseHeaders() {
  WiFiClient client;
  char endOfHeaders[] = "\r\n\r\n"; // HTTP headers end with an empty line
  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);
  if (!ok) { Serial.println("No response or invalid response!"); }
  return ok;
}

/**********************************************************
 * Function Name: RetrieveTSChannelData
 * Functionality: Receive Thingspeak channel Data
 * Notes        : Control hand up down gesture
***********************************************************/
void RetrieveTSChannelData()   // Receive data from Thingspeak
{
  int statusCode;
  long field2Data = ThingSpeak.readLongField(Channelno, FieldLamp2, APIreadkey);
  statusCode = ThingSpeak.getLastReadStatus();
  if (statusCode == 200)
  {
    Serial.println(field2Data);
  }
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
    Serial.println("v");
  }
  if(gesture == APDS9960_UP)
  {
    Serial.println("^");
  }
  if(gesture == APDS9960_LEFT)
  {
    Serial.println("<");
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
    Serial.println(">");
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
 * Function Name: thingspeakUpdateForOtherLamp
 * Functionality: Detect gesture and update server
 * Notes        :
***********************************************************/
void thingspeakUpdateForOtherLamp()
{
  int red_loc, green_loc, blue_loc;
  if(apds.readProximity()>15)
  {
    handcount++;
    if(handcount>200)
    {
      apds.enableProximity(false);
      red_loc   = pulseIn(red, HIGH);
      green_loc = pulseIn(green, HIGH);
      blue_loc  = pulseIn(blue, HIGH);
      Serial.println("RGB read value:");
      Serial.println(red_loc);
      Serial.println(green_loc);
      Serial.println(blue_loc);
      analogWrite(red,160);
      analogWrite(green,255);
      analogWrite(blue,255);

	  Serial.println("Send data to server...");
       //---------------------------------------------------------------------
       //Connect to host, host(web site) is define at top
       if(!client.connect(host, httpPort)){
         Serial.println("Connection Failed");
         delay(300);
         return; //Keep retrying until we get connected
       }

      //---------------------------------------------------------------------
        //Make GET request as pet HTTP GET Protocol format
        String ADCData;
        int Datatosend = 1;
        ADCData = String(Datatosend);   //String to interger conversion
        String Link="GET /update?api_key="+APIreadkey_str+"&field1=";  //Requeste webpage
        Link = Link + ADCData;
        Link = Link + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n";
        client.print(Link);
        delay(100);

      //---------------------------------------------------------------------
       //Wait for server to respond with timeout of 5 Seconds
       int timeout=0;
       while((!client.available()) && (timeout < 1000))     //Wait 5 seconds for data
       {
         delay(10);  //Use this with time out
         timeout++;
       }

      //---------------------------------------------------------------------
       //If data is available before time out read it.
       if(timeout < 500)
       {
           while(client.available()){
              Serial.println(client.readString()); //Response from ThingSpeak
           }
       }
       else
       {
           Serial.println("Request timeout..");
       }
       apds.enableProximity(true);
       analogWrite(red,  red_loc);
       analogWrite(green,green_loc);
       analogWrite(blue, blue_loc);
       handcount=0;
    }
  }
  else
  {
    handcount=0;
  }
}

/**********************************************************
 * Function Name: setup
 * Functionality: Arduino setup function
 * Notes        :
***********************************************************/
void setup()
{
  Serial.begin(9600);
  Server.on("/", rootPage);
  if (Portal.begin()) {
    Serial.println("HTTP server:" + WiFi.localIP().toString());
  }
  Blynk.begin(auth, ssid, pass);

  analogWrite(red,rcount);
  analogWrite(green,gcount);
  analogWrite(blue,bcount);

  pinMode(rButton, INPUT);
  pinMode(gButton, INPUT);
  pinMode(bButton, INPUT);

  if(!apds.begin()){
    Serial.println("failed to initialize device! Please check your wiring.");
  }
  else Serial.println("Gesture initialized!");

  //gesture mode will be entered once proximity mode senses something close
  apds.enableProximity(true);
  apds.enableGesture(true);
  ThingSpeak.begin(client);
  colorIdx = random(0, 200);
  delay(10000);
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
     delay(500);
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
     delay(500);
  }*/
  if (bluebutton == HIGH) // Read Blue touch sensor
  {
     bcount+=40;
     if (bcount > 1023)
     {
        bcount = 0;
     }
     analogWrite(blue,bcount);
     Serial.print("Blue Value: ");
     Serial.println(bcount);
     delay(500);
  }
  if ((pinValue) && (millis()>time_now+900000))
  {
    time_now=millis();
    mapColor(0,50,(int)getweather());
  }
  if (millis()>time_now2+20000)
  {
    time_now2=millis();
    RetrieveTSChannelData();
  }
  handleGesture();
  thingspeakUpdateForOtherLamp();
}
