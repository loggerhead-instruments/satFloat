// satFloat
// Loggerhead Instruments
// c 2019

// To Do:
// - set and test 5V supply
// - read Voltage
// - read GPS
// - control VHF power
// - set Time
// - send GPS over Iridium
// - sleep, trigger wake with timer and accelerometer
// - trigger Iridium send based on accelerometer

#include <Wire.h>
#include <RTCZero.h>
#include "LowPower.h"

// Pin Mapping Arduino Zero
//https://github.com/arduino/ArduinoCore-samd/blob/master/variants/arduino_zero/variant.cpp
#define ledGreen 5
#define iPow A0           //PA02
#define iAVAILABLE AREF   //PA03  change this t0 pin 7 (PB08 A1) or pin 8 (PB09 A2)
#define vSense A4         // PA04
#define iEnable 9         //PA07
//#define VHF A5            //PB03  not assigned -- change to 47, which is A5 PB02

#define LED_ON LOW
#define LED_OFF HIGH
#define ADXL343_ADDRESS 0x53

/* Create an rtc object */
RTCZero rtc;

int16_t accelX, accelY, accelZ;

void setup() {
  SerialUSB.begin(115200);
  analogReference(AR_DEFAULT);
  pinMode(ledGreen, OUTPUT);
  digitalWrite(ledGreen,LED_OFF);
  pinMode(vSense, INPUT);

  // Iridium setup
  pinMode(iPow, OUTPUT);
  pinMode(iEnable, OUTPUT);
  digitalWrite(iPow, LOW);
  digitalWrite(iEnable, LOW);
  
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
  SerialUSB.print(accelZ);
  SerialUSB.print(' ');
  SerialUSB.print(readVoltage());
  SerialUSB.println('V');
  delay(100);
  

}

float readVoltage(){
  float vDivider = 0.5;
  float vReg = 3.3;
  float voltage = (float) analogRead(vSense) * vReg / (vDivider * 1024.0);
  return voltage;
}
