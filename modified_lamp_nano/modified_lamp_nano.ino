#include <Arduino_APDS9960.h>

//MACROS

#define TOUCH_SENSTIVITY 102
 
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
int colorIdx       = 0;

// COLOR ARRAY

unsigned short int random_color_array[100][3] =  {{43, 985, 215}, {675, 463, 484}, {889, 976, 539}, {735, 957, 766}, {206, 674, 629}, {587, 548, 976},
                                                  {769, 498, 441}, {825, 304, 1016}, {658, 160, 970}, {566, 266, 447}, {45, 538, 628}, {437, 904, 640}, {378, 469, 737},
                                                  {624, 325, 529}, {118, 87, 497}, {421, 462, 982}, {603, 861, 473}, {940, 384, 727}, {140, 841, 378}, {25, 731, 190},
                                                  {234, 861, 815}, {175, 290, 631}, {434, 670, 852}, {91, 850, 71}, {812, 464, 132}, {293, 884, 570}, {181, 117, 827},
                                                  {23, 828, 290}, {678, 777, 441}, {425, 788, 556}, {274, 554, 388}, {778, 391, 680}, {818, 960, 541}, {0, 138, 146},
                                                  {532, 794, 792}, {540, 673, 430}, {743, 830, 262}, {436, 139, 4}, {686, 175, 864}, {405, 88, 401}, {551, 296, 403},
                                                  {408, 780, 999}, {377, 599, 512}, {452, 712, 988}, {125, 587, 42}, {260, 380, 180}, {849, 114, 970}, {762, 443, 77},
                                                  {38, 615, 764}, {442, 226, 894}, {531, 644, 789}, {985, 437, 921}, {410, 205, 613}, {125, 801, 738}, {192, 809, 175},
                                                  {692, 845, 520}, {753, 113, 994}, {162, 990, 217}, {659, 309, 889}, {739, 845, 806}, {341, 746, 95}, {462, 914, 364},
                                                  {687, 890, 252}, {481, 737, 91}, {42, 453, 737}, {911, 1015, 315}, {47, 75, 921}, {58, 149, 31}, {531, 347, 349},
                                                  {796, 255, 502}, {844, 373, 775}, {214, 798, 69}, {540, 955, 871}, {235, 375, 389}, {567, 201, 809}, {56, 657, 466},
                                                  {126, 779, 829}, {340, 405, 733}, {310, 952, 650}, {825, 278, 480}, {973, 342, 929}, {644, 604, 41}, {748, 773, 694},
                                                  {501, 96, 68}, {923, 23, 607}, {952, 540, 480}, {673, 321, 245}, {767, 150, 646}, {894, 467, 484}, {574, 958, 798},
                                                  {551, 503, 732}, {606, 893, 959}, {727, 619, 951}, {6, 928, 495}, {542, 628, 964}, {873, 239, 449}, {56, 686, 71},
                                                  {108, 25, 848}, {463, 979, 790}, {288, 405, 687}};

// FUNCTIONS

void sendTxMessage(int code, bool touch)
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
  if (touch)
  { 
    TxData += "," + String(rcount) + "," + String(gcount) + "," + String(bcount);
  }
  else
  {
    TxData += "," + String((int)random_color_array[colorIdx][0]) + "," + String((int)random_color_array[colorIdx][1]) + "," + String((int)random_color_array[colorIdx][2]);
  }
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

  colorIdx = random(0, 99);
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
        sendTxMessage(2, false);
        lastUpdate = millis();
        break;

      case GESTURE_DOWN:
        gestureVar = 1;
        sendTxMessage(2, false);
        lastUpdate = millis();
        break;

      case GESTURE_LEFT:
        gestureVar = 2;
        sendTxMessage(2, false);
        if(colorIdx<=0)
        {
          colorIdx = 99;
        }
        else
        {
          colorIdx--;
        }
        lastUpdate = millis();
        break;

      case GESTURE_RIGHT:
        gestureVar = 3;
        sendTxMessage(2,false);
        if(colorIdx>=99)
        {
          colorIdx = 0;
        }
        else
        {
          colorIdx++;
        }
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
    sendTxMessage(0, true);
  }
  
  redbutton   = digitalRead(rButton);
  greenbutton = digitalRead(gButton);
  bluebutton  = digitalRead(bButton);

  if (redbutton == HIGH) // Read Red touch sensor
  {
     rcount+=TOUCH_SENSTIVITY;
     if (rcount > 1023)
     {
        rcount = 0;
     }
     sendTxMessage(1,true);
     lastUpdate = millis();
     //Serial.print("Red Value: ");
     //Serial.println(rcount);
     delay(1000);
  }
  if (greenbutton == HIGH) // Read Green touch sensor
  {
     gcount+=TOUCH_SENSTIVITY;
     if (gcount > 1023)
     {
        gcount = 0;
     }
     sendTxMessage(1,true);
     lastUpdate = millis();
     //Serial.print("Green Value: ");
     //Serial.println(gcount);
     delay(1000);
  }
  if (bluebutton == HIGH) // Read Blue touch sensor
  {
     bcount+=TOUCH_SENSTIVITY;
     if (bcount > 1023)
     {
        bcount = 0;
     }
     sendTxMessage(1,true);
     lastUpdate = millis();
     //Serial.print("Blue Value: ");
     //Serial.println(bcount);
     delay(1000);
  }
}
