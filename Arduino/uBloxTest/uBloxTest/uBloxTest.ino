#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_FeatherOLED.h>

#define displayLine1 0
#define displayLine2 9
#define displayLine3 18
#define displayLine4 27
Adafruit_FeatherOLED display = Adafruit_FeatherOLED();
#define BOTTOM 25

long timeStart;

void setup() {

  Wire.begin();
  Serial1.begin(9600);
  display.init();
  timeStart = millis();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

}

void loop() {
  byte incomingByte;
  display.setCursor(10,0);
  while (Serial1.available() > 0) {    
        incomingByte = Serial1.read();
        Serial.write(incomingByte);
        display.write(incomingByte);
        
  }
  display.setCursor(0,0);
  display.println((millis() - timeStart) / 1000);
  display.display();
  delay(1000);
  display.clearDisplay();
}
