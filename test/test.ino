#include <Wire.h>
#include <APDS9960_NonBlocking.h>

#define DEBUG_GENERAL      // Undefine this for general debug information via the serial port. 
#define GESTURE_SENSOR_I2C_ADDRESS 0x3F

boolean bGestureAvailableFlag = false;
uint8_t uiGestureValue = APDS9960_GVAL_NONE;

APDS9960_NonBlocking gestureSensor(GESTURE_SENSOR_I2C_ADDRESS);

void setup (void)
{
  #ifdef DEBUG_GENERAL
  Serial.begin(115200);           // start serial for output
  Serial.println("Serial port Active");
  #endif  
  bGestureAvailableFlag = false;
  uiGestureValue = APDS9960_GVAL_NONE;
  
  Wire.begin();
  if (gestureSensor.init()) { ;
      #ifdef DEBUG_GENERAL
      Serial.println("APDS9960 initialised");
      #endif  
  } else { ;
      #ifdef DEBUG_GENERAL
      Serial.println("APDS9960 not responding");
      #endif  
  }
  delay(1000);
}

  

void readAPDS9960(boolean *bGestureAvailableFlag, uint8_t *uiGestureValue)
{
  *bGestureAvailableFlag = false;
  if (gestureSensor.gestureAvailable()) {
    *uiGestureValue = gestureSensor.read();
    gestureSensor.gestureClear();
    *bGestureAvailableFlag = true;
  }
  //delay(1000);
}


void loop (void)
{
  char cArray[30];

  readAPDS9960(&bGestureAvailableFlag, &uiGestureValue);
  if (bGestureAvailableFlag) {
    bGestureAvailableFlag = false;

    switch(uiGestureValue) {
      case APDS9960_GVAL_NONE :
        sprintf(cArray,"None");
        break;
      case APDS9960_GVAL_ERROR :
        sprintf(cArray,"Error");
        break;
      case APDS9960_GVAL_UP :
        sprintf(cArray,"Up");
        break;
      case APDS9960_GVAL_DOWN :
        sprintf(cArray,"Down");
        break;
      case APDS9960_GVAL_LEFT :
        sprintf(cArray,"Left");
        break;
      case APDS9960_GVAL_RIGHT :
        sprintf(cArray,"Right");
        break;
      case APDS9960_GVAL_NEAR :
        sprintf(cArray,"Near");
        break;
      case APDS9960_GVAL_FAR :
        sprintf(cArray,"Far");
        break;
    }
    #ifdef DEBUG_GENERAL
    Serial.println(cArray);
    #endif  
  }
}
