#include <Arduino_APDS9960.h>

//GPIO PIN MACROS

// GLOBAL VARIABLE

int redbutton                = 0;
int greenbutton              = 0;
int bluebutton               = 0;
int gcount                   = 0;
int bcount                   = 0;
int rcount                   = 0;
int proximity                = 0;
int gestureVar                  = 0;
int r = 0, g = 0, b = 0;
unsigned long lastUpdate     = 0;

const int rButton  = 6;
const int gButton  = 7;
const int bButton  = 8;

// FUNCTIONS

void sendTxMessage(int code)
{
 /*
 * CODE SEND BEFORE TX DATA
 * 0 - RGB DATA
 * 1 - Touch Button Trigger
 * 2 - Gesture Trigger
 *     0 - UP
 *     1 - DOWN
 *     2 - LEFT
 *     3 - RIGHT 
 */
 
  String TxData;
  TxData = String(code) + "," + String(r) + "," + String(g) + "," + String(b); 
  TxData += "," + String(rcount) + "," + String(gcount) + "," + String(bcount);
  TxData += "," + String(gestureVar);
  TxData += "," + String(proximity);
  Serial.println(TxData);
  delay(1000);
}

void setup() 
{
  Serial.begin(9600);
  while (!Serial); // Wait for serial monitor to open

  if (!APDS.begin()) {
    //Serial.println("Error initializing APDS9960 sensor.");
    while (true); // Stop forever
  }
  APDS.setGestureSensitivity(90);

  pinMode(rButton, INPUT);
  pinMode(gButton, INPUT);
  pinMode(bButton, INPUT);
}

void loop() {
  
  // Check if a proximity reading is available.
  if (APDS.proximityAvailable()) {
    proximity = APDS.readProximity();
  }

  // check if a gesture reading is available
  if (APDS.gestureAvailable()) {
    int gesture = APDS.readGesture();
    switch (gesture) {
      case GESTURE_UP:
        gestureVar = 0;
        sendTxMessage(2);
        lastUpdate = millis();
        break;

      case GESTURE_DOWN:
        gestureVar = 1;
        sendTxMessage(2);
        lastUpdate = millis();
        break;

      case GESTURE_LEFT:
        gestureVar = 2;
        sendTxMessage(2);
        lastUpdate = millis();
        break;

      case GESTURE_RIGHT:
        gestureVar = 3;
        sendTxMessage(2);
        lastUpdate = millis();
        break;

      default:
        // ignore
        break;
    }
  }

  // check if a color reading is available
  if (APDS.colorAvailable()) {
    APDS.readColor(r, g, b);
  }

  if (millis() - lastUpdate > 10000) {
    lastUpdate = millis();
    sendTxMessage(0);
  }
  
  redbutton   = digitalRead(rButton);
  greenbutton = digitalRead(gButton);
  bluebutton  = digitalRead(bButton);

  if (redbutton == HIGH) // Read Red touch sensor
  {
     rcount+=40;
     if (rcount > 1023)
     {
        rcount = 0;
     }
     sendTxMessage(1);
     lastUpdate = millis();
     //Serial.print("Red Value: ");
     //Serial.println(rcount);
     delay(300);
  }
  if (greenbutton == HIGH) // Read Green touch sensor
  {
     gcount+=80;
     if (gcount > 1023)
     {
        gcount = 0;
     }
     sendTxMessage(1);
     lastUpdate = millis();
     //Serial.print("Green Value: ");
     //Serial.println(gcount);
     delay(300);
  }
  if (bluebutton == HIGH) // Read Blue touch sensor
  {
     bcount+=40;
     if (bcount > 1023)
     {
        bcount = 0;
     }
     sendTxMessage(1);
     lastUpdate = millis();
     //Serial.print("Blue Value: ");
     //Serial.println(bcount);
     delay(300);
  }
}
