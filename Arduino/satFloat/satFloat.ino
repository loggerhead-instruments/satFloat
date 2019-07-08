// satFloat
// Loggerhead Instruments
// c 2019

// Iridium ISU module needs to be configured for 3-wire (UART) operation
// Configuration is done using serial connection (e.g. FTDI board)
// Connections: TX-TX, RX-RX, DTR-DTR, CTS-CTS, GND-SG (signal ground)
// AT&D0   (ignore DTR)
// AT&K0   (ignore CTS)
// AT&W0   (store active configuration to memory)
// AT&Y0   (designate as default reset profile)

// Commands must have a carriage return \r, not a line feed
// "AT\r"

// To Do:
// - send less often when voltage gets low

// current draw around 0.4 mA in monitoring mode

#include <Wire.h>
#include <RTCZero.h>
#include "LowPower.h"
#include "wiring_private.h" // pinPeripheral() function
#include "IridiumSBD.h"

// DEV SETTINGS
int printDiags = 1;
boolean sendIridium = 1;
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
#define iPow A0           // PA02
#define iAVAILABLE A2     // PA03  change this t0 pin 7 (PB08 A1) or pin 8 (PB09 A2)
#define vSense A4         // PA04
#define iEnable 9         // PA07
#define VHF A5            // PB02
#define gpsEnable 8       // PA06
#define ACCELINT1 7       // Accelerometer interrupt 1

#define LED_ON LOW
#define LED_OFF HIGH
#define ADXL343_ADDRESS 0x53

/* Create an rtc object */
RTCZero rtc;

// Iridium
IridiumSBD modem(Serial1, iEnable);
int sigStrength;

// GPS
float latitude = 0.0;
float longitude = 0.0;
char latHem, lonHem;
int gpsYear = 19, gpsMonth = 4, gpsDay = 17, gpsHour = 22, gpsMinute = 5, gpsSecond = 0;
int goodGPS = 0;
long gpsTimeOutThreshold = 600000;

/* Change these values to set the current initial time and date */
volatile byte second = 0;
volatile byte minute = 0;
volatile byte hour = 17;
volatile byte day = 1;
volatile byte month = 1;
volatile byte year = 19;

int16_t accelX, accelY, accelZ;

void setup() {
  delay(10000);
  SerialUSB.begin(115200);
  delay(1000);
  SerialUSB.println("OpenSat");
  delay(1000);

  analogReference(AR_DEFAULT);
  pinMode(ledGreen, OUTPUT);

  pinMode(ACCELINT1, INPUT);
  
  pinMode(vSense, INPUT);
  pinMode(gpsEnable, OUTPUT);
  digitalWrite(gpsEnable, LOW);

// Iridium setup
  pinMode(iPow, OUTPUT);
  pinMode(iEnable, OUTPUT);
  digitalWrite(iPow, HIGH);
  digitalWrite(iEnable, HIGH);
  delay(3000);
  digitalWrite(ledGreen, LED_ON);
  
  Serial1.begin(19200);  //Iridium

// Quick Iridium modem test
  Serial1.write('A');
  Serial1.write('T');
  Serial1.write(0x0D);

  delay(1000);
  while(Serial1.available()){
    byte data = Serial1.read();
    SerialUSB.write(data);
  }
  
  //modem.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE);
  int result = modem.begin();
  if (result != ISBD_SUCCESS)
  {
    SerialUSB.print("Begin failed: error ");
    SerialUSB.println(result);
    if (result == ISBD_NO_MODEM_DETECTED)
      SerialUSB.println("No modem detected: check wiring.");
  }
  modem.getSignalQuality(sigStrength); // update Iridium modem strength
  SerialUSB.print("Signal Strength:");
  SerialUSB.println(sigStrength);
  digitalWrite(iPow, LOW);  // power down Iridium
  digitalWrite(iEnable, LOW);

  // GPS Setup
  Serial2.begin(9600);
  // Assign pins 10 & 11 SERCOM functionality
  pinPeripheral(10, PIO_SERCOM);
  pinPeripheral(11, PIO_SERCOM);

  rtc.begin();
 // gpsGetTimeLatLon();
//  if(goodGPS){
//    rtc.setTime(gpsHour, gpsMinute, gpsSecond);
//    rtc.setDate(gpsDay, gpsMonth, gpsYear);
//  }
//  else{
//    while(1){
//      digitalWrite(ledGreen, LED_ON);
//      delay(150);
//      digitalWrite(ledGreen, LED_OFF);
//      delay(50);
//    }
//  }
//  
  Wire.begin();
  Wire.setClock(400);  // set I2C clock to 400 kHz
  accelInit(ADXL343_ADDRESS, 0, 0);
  attachInterrupt(ACCELINT1, wakeUp, HIGH);
  Read_Accel_Int(ADXL343_ADDRESS);
}

int intCounter = 0;  // used to count number of interrupts; so only send Iridium every 10 minutes
// interrupt is every 5 seconds
// so 10 min = 600 s = 120 interrupts
int nInterrupts = 12;
void loop() {
  readAccel(ADXL343_ADDRESS);
 // printTime();
//  SerialUSB.print(accelX);
//  SerialUSB.print(' ');
//  SerialUSB.print(accelY);
//  SerialUSB.print(' ');
//  SerialUSB.print(accelZ);
//  SerialUSB.print(' ');
//  SerialUSB.print(readVoltage());
//  SerialUSB.println('V');

  intCounter++;
  // tag is mostly vertical; try to get GPS and send
  if(accelX>130 & intCounter>nInterrupts){
    intCounter = 0;
//    if(printDiags){
//      USBDevice.attach();
//      while(!SerialUSB);
//    }
    digitalWrite(iPow, HIGH);  // Iridium on
    digitalWrite(iEnable, HIGH);
    gpsGetTimeLatLon();
    makeDataPacket();
    if(sendIridium){
      sendDataPacket();
    }
    digitalWrite(iPow, LOW);  // Iridium off
    digitalWrite(iEnable, LOW);
//    if(printDiags){
//      USBDevice.detach();
//    }
  }

  digitalWrite(ledGreen, LED_OFF);
  LowPower.standby();
  // .... Sleeping ... //

}

float readVoltage(){
  float vDivider = 0.5;
  float vReg = 3.3;
  float voltage = (float) analogRead(vSense) * vReg / (vDivider * 1024.0);
  return voltage;
}

void printTime(){
  getTime();
  SerialUSB.print(year); SerialUSB.print("-");
  SerialUSB.print(month); SerialUSB.print("-");
  SerialUSB.print(day); SerialUSB.print(" ");
  SerialUSB.print(hour); SerialUSB.print(":");
  SerialUSB.print(minute); SerialUSB.print(":");
  SerialUSB.print(second); SerialUSB.print(" ");
}

void getTime(){
  day = rtc.getDay();
  month = rtc.getMonth();
  year = rtc.getYear();
  hour = rtc.getHours();
  minute = rtc.getMinutes();
  second = rtc.getSeconds();
}

void wakeUp(){
  digitalWrite(ledGreen, LED_ON);
  Read_Accel_Int(ADXL343_ADDRESS);
}
