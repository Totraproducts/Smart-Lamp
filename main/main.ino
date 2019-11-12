// Includes
#define BLYNK_PRINT Serial

#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <AutoConnect.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <esp8266httpclient.h>
ESP8266WebServer Server;
AutoConnect      Portal(Server);

const int button=5;
#define red D6
#define green D8
#define blue D7

void rootPage() 
{
  char content[] = "TOTRA PRODUCTS";
  Server.send(200, "text/plain", content);
}

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "E5ow8NqRrM5i1uO-fIdPw--ViQDnrfze";
 
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "idk";
char pass[] = "idk";

int count=0;
int redbutton = 0;
int pinValue=0;
unsigned long time_now=0;
unsigned long time_now2=0;

// defines pins numbers for distance sensor

const int trigPin = 0;  //D3
const int echoPin = 2;  //D4

// defines variables for distance sensor
long duration;
int distance;

char   host[]      = "api.thingspeak.com"; // ThingSpeak address
String APIkey      = "875199";             // Thingspeak Read Key, works only if a PUBLIC viewable channel
String APIreadkey  = "LNMGX4DFAJK0MY5B";   // Thingspeak Read Key, works only if a PUBLIC viewable channel
const int httpPort = 80;

const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server

BLYNK_WRITE(V0)
{
  float temp;
  pinValue = param.asInt();
  // String i = param.asStr();
  // double d = param.asDouble();
  Serial.print("V1 Slider value is: ");
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

void mapColor(float temp)
{
  int redc,bluec;
  redc = map(temp, 5, 45, 0, 1023);
  bluec = 1023 - red;
  analogWrite(red,redc);
  analogWrite(blue,bluec);
}

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
      Serial.println(temp);
    }
    http.end();   //Close connection
  }
  return temp;
}

int dis_cal_cm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
 
  distance= duration*0.034/2;
  return(distance);
}

void gesture()
{
  int r_val, gb_val;
  distance = dis_cal_cm();
  if((distance>2)&&(distance<30))
  {
    gb_val = map(distance, 3, 29, 0, 1023);
    r_val = map(gb_val, 0, 1023, 0, 420);
    Serial.print("R :");
    Serial.println(r_val);
    Serial.print("GB :");
    Serial.println(gb_val);
    Serial.print("Distance :");
    Serial.println(distance);
    analogWrite(red,  r_val);
    analogWrite(green,gb_val);
    analogWrite(blue, gb_val);
    delay(100);
  }
}

void RetrieveTSChannelData() {  // Receive data from Thingspeak
  WiFiClient client;
  static char responseBuffer[1024]; // Buffer for received data
  WiFiServer server(80);
  client = server.available(); 
  if (!client.connect(host, httpPort)) { 
    Serial.println("connection failed"); 
    return; 
  } 
  String url = "/channels/" + APIkey; // Start building API request string
  //GET /channels/CHANNEL_ID/feeds.json?api_key=<your API key>&results=2
  #ifdef PRIVATE 
  url += "/fields/1.json?results=1";  // 1 is the results request number, so 5 are returned, 1 woudl return the last result received
  #else
  url += "/fields/1.json?api_key="+APIreadkey+"&results=1";  // 1 is the results request number, so 1 are returned, 1 woudl return the last result received
  #endif
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  Serial.println("\n\r"+String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  while (!skipResponseHeaders());                      // Wait until there is some data and skip headers
  while (client.available()) {                         // Now receive the data
    String line = client.readStringUntil('\n');
    if ( line.indexOf('{',0) >= 0 ) {                  // Ignore data that is not likely to be JSON formatted, so must contain a '{'
      Serial.println(line);                            // Show the text received
      line.toCharArray(responseBuffer, line.length()); // Convert to char array for the JSON decoder
      decodeJSON(responseBuffer);                      // Decode the JSON text
    }
  }
  client.stop();
}

bool skipResponseHeaders() { 
  WiFiClient client;
  char endOfHeaders[] = "\r\n\r\n"; // HTTP headers end with an empty line 
  client.setTimeout(HTTP_TIMEOUT); 
  bool ok = client.find(endOfHeaders); 
  if (!ok) { Serial.println("No response or invalid response!"); } 
  return ok; 
} 

bool decodeJSON(char *json) {
  StaticJsonDocument<1024> jsonBuffer;
  char *jsonstart = strchr(json, '{'); // Skip characters until first '{' found and ignore length, if present
  if (jsonstart == NULL) {
    Serial.println("JSON data missing");
    return false;
  }
  json = jsonstart;
  JsonObject& root = jsonBuffer.parseObject(json); // Parse JSON
  if (!root.success()) {
    Serial.println(F("jsonBuffer.parseObject() failed"));
    return false;
  }
  JsonObject& root_data = root["channel"]; // Begins and ends within first set of { }
  String id   = root_data["id"];
  String name = root_data["name"];
  String field1_name = root_data["field1"];
  String datetime    = root_data["updated_at"];
  Serial.println("\n\n Channel id: "+id+" Name: "+ name);
  Serial.println(" Readings last updated at: "+datetime);
  
  for (int result = 0; result < 1; result++){
    JsonObject& channel = root["feeds"][result]; // Now we can read 'feeds' values and so-on
    String entry_id     = channel["entry_id"];
    String field1value  = channel["field1"];
    Serial.print(" Field1 entry number ["+entry_id+"] had a value of: ");
    Serial.println(field1value);
  }
}

void setup()
{
  Serial.begin(9600);
  Server.on("/", rootPage);
  if (Portal.begin()) {
    Serial.println("HTTP server:" + WiFi.localIP().toString());
  }
  Blynk.begin(auth, ssid, pass);

  analogWrite(red,count);
  analogWrite(green,count);
  analogWrite(blue,count);
  
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
}

void loop()
{
  redbutton = digitalRead(button);
  Blynk.run();
  Portal.handleClient();
  if (redbutton == HIGH) 
  {
     count+=40;
     if (count > 1023)
     {
        count = 0;
     }
     analogWrite(red,count);
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
    RetrieveTSChannelData();
  }
  gesture();
}