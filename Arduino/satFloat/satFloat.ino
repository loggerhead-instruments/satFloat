// satFloat
// Loggerhead Instruments
// c 2019

// To Do:
// - read GPS over second interface
// - control VHF power
// - set Time
// - send GPS over Iridium
// - sleep, trigger wake with timer and accelerometer

#include <Wire.h>
#include <RTCZero.h>
#include "LowPower.h"
#include "wiring_private.h" // pinPeripheral() function

// DEV SETTINGS
int printDiags = 1;

// Define Serial2 to talk to GPS
// https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial
Uart Serial2 (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

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

// GPS
float latitude = 0.0;
float longitude = 0.0;
char latHem, lonHem;
int gpsYear = 19, gpsMonth = 4, gpsDay = 17, gpsHour = 22, gpsMinute = 5, gpsSecond = 0;
int goodGPS = 0;
long gpsTimeOutThreshold = 600000;

int16_t accelX, accelY, accelZ;

void setup() {
  SerialUSB.begin(115200);
  
  analogReference(AR_DEFAULT);
  pinMode(ledGreen, OUTPUT);
  digitalWrite(ledGreen,LED_OFF);
  pinMode(vSense, INPUT);

  // GPS Setup
  Serial2.begin(9600);
  // Assign pins 10 & 11 SERCOM functionality
  pinPeripheral(10, PIO_SERCOM);
  pinPeripheral(11, PIO_SERCOM);

  // Iridium setup
  pinMode(iPow, OUTPUT);
  pinMode(iEnable, OUTPUT);
  digitalWrite(iPow, LOW);
  digitalWrite(iEnable, LOW);
  
  delay(10000);

  rtc.begin();
  SerialUSB.println("Loggerhead SatFloat");
  gpsGetTimeLatLon();
  if(goodGPS){
    rtc.setTime(gpsHour, gpsMinute, gpsSecond);
    rtc.setDate(gpsDay, gpsMonth, gpsYear);
  }
  else{
    while(1){
      digitalWrite(ledGreen, LED_ON);
      delay(150);
      digitalWrite(ledGreen, LED_OFF);
      delay(50);
    }
  }
  
  Wire.begin();
  Wire.setClock(400);  // set I2C clock to 400 kHz
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
