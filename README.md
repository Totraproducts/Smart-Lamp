# Smart_Lamp

![For Loop](https://drive.google.com/uc?export=view&id=1pKhNEhpKTEP9s6ogW-5lVDfEwQf6RmGy)
## IOT based RGB lamp powered by esp8266
There is need to modify Adafruit library.
Change _**...\Arduino\libraries\Adafruit_APDS9960_Library\Adafruit_APDS9960.cpp**_ file

In function readGesture() of class Adafruit_APDS9960 the infinite while loop is replaced with for loop with 9 itteration.

Code Snippit

![For Loop](https://drive.google.com/uc?export=view&id=1_Mugt_VH-SxxUoBaJzA1pB9ljT75HNpe)

The Modified function should be look like that:
```
/*!
 *  @brief  Reads gesture
 *  @return Received gesture (APDS9960_DOWN APDS9960_UP, APDS9960_LEFT
 *          APDS9960_RIGHT)
 */
uint8_t Adafruit_APDS9960::readGesture() {
  uint8_t toRead, bytesRead;
  uint8_t buf[256];
  unsigned long t = 0;
  uint8_t gestureReceived;
  int i;
  int up_down_diff, left_right_diff;
  //while (1) {
  for (i=0;i<9;i++) {
    up_down_diff = 0;
    left_right_diff = 0;
    gestureReceived = 0;
    if (!gestureValid())
      return 0;

    delay(30);
    toRead = this->read8(APDS9960_GFLVL);

    // bytesRead is unused but produces sideffects needed for readGesture to work
    bytesRead = this->read(APDS9960_GFIFO_U, buf, toRead);

    if (abs((int)buf[0] - (int)buf[1]) > 13)
      up_down_diff += (int)buf[0] - (int)buf[1];

    if (abs((int)buf[2] - (int)buf[3]) > 13)
      left_right_diff += (int)buf[2] - (int)buf[3];

    if (up_down_diff != 0) {
      if (up_down_diff < 0) {
        if (DCount > 0) {
          gestureReceived = APDS9960_UP;
        } else
          UCount++;
      } else if (up_down_diff > 0) {
        if (UCount > 0) {
          gestureReceived = APDS9960_DOWN;
        } else
          DCount++;
      }
    }

    if (left_right_diff != 0) {
      if (left_right_diff < 0) {
        if (RCount > 0) {
          gestureReceived = APDS9960_LEFT;
        } else
          LCount++;
      } else if (left_right_diff > 0) {
        if (LCount > 0) {
          gestureReceived = APDS9960_RIGHT;
        } else
          RCount++;
      }
    }
    if (up_down_diff != 0 || left_right_diff != 0)
    {
      t = millis();
    }

    if (gestureReceived || millis() - t > 300) {
      resetCounts();
      return gestureReceived;
    }
  }
}
```
