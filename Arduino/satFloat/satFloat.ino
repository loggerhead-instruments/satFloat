// satFloat
// Loggerhead Instruments
// c 2019

// To Do:
// - read accelerometer
// - light LED when vertical
// - read GPS
// - set Time
// - send GPS over Iridium
// - sleep, trigger wake with timer and accelerometer
// - trigger Iridium send based on accelerometer

#include <Wire.h>
#include <RTCZero.h>
#include "LowPower.h"

const int ledGreen = 5;


#define LED_ON LOW
#define LED_OFF HIGH
#define ADXL343_ADDRESS 0x53

/* Create an rtc object */
RTCZero rtc;

int16_t accelX, accelY, accelZ;

void setup() {
  SerialUSB.begin(115200);
  pinMode(ledGreen, OUTPUT);
  digitalWrite(ledGreen,LED_OFF);
  delay(10000);
  Wire.begin();
  Wire.setClock(400);  // set I2C clock to 400 kHz
  rtc.begin();

  accelInit(ADXL343_ADDRESS, 25, 0, 0);

}

void loop() {
  readAccel(ADXL343_ADDRESS);
  SerialUSB.print(accelX);
  SerialUSB.print(' ');
  SerialUSB.print(accelY);
  SerialUSB.print(' ');
  SerialUSB.println(accelZ);
  delay(100);
  

}
