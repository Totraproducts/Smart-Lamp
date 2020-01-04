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

ESP8266WebServer Server;
AutoConnect      Portal(Server);
WiFiClient       client;

/***************** GLOBAL VARIABLES *******************/

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).

char auth[]            = "E5ow8NqRrM5i1uO-fIdPw--ViQDnrfze";

// Your WiFi credentials.
char ssid[]            = "idk";
char pass[]            = "idk";
int rcount             = 0;
int gcount             = 0;
int bcount             = 0;
int redbutton          = 0;
int greenbutton        = 0;
int bluebutton         = 0;
int pinValue           = 0;
unsigned long time_now = 0;
unsigned long time_now2= 0;
unsigned long time_now3= 0;
const int FieldLamp1   = 1;
const int FieldLamp2   = 2;

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
int handcount                    =0;

Adafruit_APDS9960 apds;

/***************** GPIO PIN DEFINES *******************/

const int rButton = 14;   //GPIO2 / TxD1
const int gButton = 0;    //GPIO0 / D3
const int bButton = 16;    //GPIO14 / SLCK

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
void mapColor(float temp)
{
  int redc,bluec;
  redc = map(temp, 5, 45, 0, 1023);
  bluec = 1023 - red;
  analogWrite(red,redc);
  analogWrite(blue,bluec);
}

/**********************************************************
 * Function Name: BLYNK_WRITE
 * Functionality: Exchange any data between Blynk app and your hardware
 * Notes        : Vn is virtual port
***********************************************************/
BLYNK_WRITE(V0)
{
  float temp;
  pinValue = param.asInt();
  // String i = param.asStr();
  // double d = param.asDouble();
  Serial.print("Weather Button value is: ");
  Serial.println(pinValue);
  if(pinValue)
  {
    time_now=0;
    temp = getweather();
    mapColor(temp);
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
 * Function Name: decodeJSON
 * Functionality: Decode the json text
 * Notes        :
***********************************************************/
/*bool decodeJSON(char json[]) {
  StaticJsonDocument<1024> doc;
  char *jsonstart = strchr(json, '{'); // Skip characters until first '{' found and ignore length, if present
  if (jsonstart == NULL) {
    Serial.println("JSON data missing");
    return false;
  }
  //json = jsonstart;
  auto error = deserializeJson(doc, json);   // Parse JSON
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return false;
  }
  StaticJsonDocument<512> doc2;  // Begins and ends within first set of { }
  error = deserializeJson(doc2, doc["channel"]);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return false;
  }
  String id   = doc2["id"];
  String name = doc2["name"];
  String field1_name = doc2["field1"];
  String datetime    = doc2["updated_at"];
  Serial.println("\n\n Channel id: "+id+" Name: "+ name);
  Serial.println(" Readings last updated at: "+datetime);

  for (int result = 0; result < 1; result++){
    StaticJsonDocument<200> doc3;
    auto error = deserializeJson(doc3, doc["feeds"][result]); // Now we can read 'feeds' values and so-on
    String entry_id     = doc3["entry_id"];
    String field1value  = doc3["field1"];
    Serial.print(" Field1 entry number ["+entry_id+"] had a value of: ");
    Serial.println(field1value);
  }
}*/

/**********************************************************
 * Function Name: RetrieveTSChannelData
 * Functionality: Receive Thingspeak channel Data
 * Notes        : Control hand up down gesture
***********************************************************/
void RetrieveTSChannelData() {  // Receive data from Thingspeak
  WiFiClient client;
  static char responseBuffer[1024]; // Buffer for received data
  WiFiServer server(80);
  client = server.available();
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  String url = "/channels/" + Channelno_str; // Start building API request string
  //GET /channels/CHANNEL_ID/feeds.json?api_key=<your API key>&results=2
  #ifdef PRIVATE
  url += "/fields/1.json?results=1";  // 1 is the results request number, so 5 are returned, 1 woudl return the last result received
  #else
  url += "/fields/1.json?api_key="+APIreadkey_str+"&results=1";  // 1 is the results request number, so 1 are returned, 1 woudl return the last result received
  #endif
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  Serial.println("\n\r"+String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  while (!skipResponseHeaders());                      // Wait until there is some data and skip headers
  while (client.available()) {                         // Now receive the data
    String line = client.readStringUntil('\n');
    if ( line.indexOf('{',0) >= 0 ) {                  // Ignore data that is not likely to be JSON formatted, so must contain a '{'
      Serial.println(line);                            // Show the text received
      line.toCharArray(responseBuffer, line.length()); // Convert to char array for the JSON decoder
      //decodeJSON(responseBuffer);                      // Decode the JSON text
      Serial.println(responseBuffer);
    }
  }
  client.stop();
}
void RetrieveTSChannelData2()   // Receive data from Thingspeak
{
  int statusCode;
  long field2Data = ThingSpeak.readLongField(Channelno, FieldLamp2, APIreadkey);
  statusCode = ThingSpeak.getLastReadStatus();
  if (statusCode == 200)
  {
    Serial.println("True IF condition");
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
  }
  if(gesture == APDS9960_RIGHT)
  {
    Serial.println(">");
  }
}

/**********************************************************
 * Function Name: thingspeakUpdateForOtherLamp
 * Functionality: Detect gesture and update server
 * Notes        :
***********************************************************/
void thingspeakUpdateForOtherLamp()
{
  if(apds.readProximity())
  {
    handcount++;
    if(handcount>200)
    {
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
    mapColor(getweather());
  }
  if (millis()>time_now2+10000)
  {
    time_now2=millis();
    RetrieveTSChannelData2();
  }

  handleGesture();
  thingspeakUpdateForOtherLamp();
}
