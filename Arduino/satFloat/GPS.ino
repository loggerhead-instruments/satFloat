
#define maxChar 256
char gpsStream[maxChar];
int streamPos;
volatile boolean endGpsLog;


int gps(byte incomingByte){
  char temp2[2];
  char temp3[3];
  char temp5[5];
  char temp7[7];
  char temp12[12];
  // check for start of new message, which means should have complete message
  // if a $, start it at Pos 0, and continue until next G
  if(incomingByte=='$') {
   // Serial.print("String position:");
   // Serial.println(streamPos);
   // Serial.println(gpsStream);
    //process last message
    if(streamPos > 10){
      // OriginGPS
      // $GNRMC,134211.000,A,2715.5428,N,08228.7924,W,1.91,167.64,020816,,,A*62
      // Adafruit GPS
      // $GPRMC,222250.000,A,2716.6201,N,08227.4996,W,1.01,301.49,250117,,,A*7C
      char rmcCode[6 + 1];
      char rmcTime[12]; //           225446       Time of fix 22:54:46 UTC
      char rmcValid[2]; //           A            Navigation receiver warning A = OK, V = warning
      float rmcLat; //           4916.45,N    Latitude 49 deg. 16.45 min North
      char rmcLatHem[2];
      float rmcLon; //           12311.12,W   Longitude 123 deg. 11.12 min West
      char rmcLonHem[2];
      float rmcSpeed; //           000.5        Speed over ground, Knots
      float rmcCourse;//           054.7        Course Made Good, True
      char rmcDate[6 + 1];//           191194       Date of fix  19 November 1994
      float rmcMag;//           020.3,E      Magnetic variation 20.3 deg East
      char rmcMagHem[2];
      char rmcChecksum[4 + 1]; //           *68          mandatory checksum

      // check for end of log dump  $PMTKLOX,2*47
      if(gpsStream[1]=='P' & gpsStream[2]=='M' &  gpsStream[3]=='T' &  gpsStream[4]=='K' &  gpsStream[5]=='L'  
      & gpsStream[6]=='O' &  gpsStream[7]=='X' &  gpsStream[8]==',' &  gpsStream[9]=='2' & gpsStream[10]=='*' 
      & gpsStream[11]=='4' & gpsStream[12]=='7'){
        endGpsLog = 1;
      }


      // UBLOX example: $GNRMC,184531.00,A,2715.56334,N,08228.78980,W,0.048,,180619,,,D*70

      if(gpsStream[1]=='G' & gpsStream[2]=='N' &  gpsStream[3]=='R' &  gpsStream[4]=='M' &  gpsStream[5]=='C'){
       char temp[streamPos + 1];
       //char temp[100];
       const char s[2] = ",";
       char *token;

       if(printDiags){
         Serial.println("gpsStream");
         Serial.println(gpsStream);
         Serial.println();
       }



       char splitStr[13][15];
       int j = 0;
       int k = 0;
       // parse GPS Stream
       for(int i=0; i<streamPos; i++){
        //Serial.print("NextVal:");
        //Serial.println(gpsStream[i]);
        if(gpsStream[i]!=',') 
          splitStr[j][k] = gpsStream[i];
        else{
          splitStr[j][k] = '\0';
          //Serial.println(splitStr[j]);
          k = -1;  // so ends up being 0 after k++
          j++;
        }
        k++;
       }
       splitStr[j][k] = '\0'; // terminate last one

//        for(int i=0; i<12; i++){
//          Serial.println(splitStr[i]);
//        }
       
       sscanf(splitStr[1], "%2d%2d%2d", &gpsHour, &gpsMinute, &gpsSecond); 
       sscanf(splitStr[2], "%s", rmcValid);
       sscanf(splitStr[3], "%f", &rmcLat);   
       sscanf(splitStr[4], "%s", rmcLatHem);
       sscanf(splitStr[5], "%f", &rmcLon);   
       sscanf(splitStr[6], "%s", rmcLonHem);
       sscanf(splitStr[9], "%2d%2d%2d", &gpsDay, &gpsMonth, &gpsYear);

       if(printDiags){
        Serial.print("rmcCode:");
        Serial.println(rmcCode);

        Serial.print("rmcValid:");
        Serial.println(rmcValid);

        Serial.print("rmcLat:");
        Serial.println(rmcLat, 6);

        Serial.print("rmcLatHem:");
        Serial.println(rmcLatHem);

        Serial.print("rmcLon:");
        Serial.println(rmcLon, 6);

        Serial.print("rmcLonHem:");
        Serial.println(rmcLonHem);

        Serial.print("Day-Month-Year:");
        Serial.print(gpsDay); Serial.print("-");
        Serial.print(gpsMonth);  Serial.print("-");
        Serial.println(gpsYear);

        Serial.print("rmcCheckSum:");     
        Serial.println(rmcChecksum);          
       }
       

        float tempLatitude, tempLongitude;
        if(rmcValid[0]=='A'){
           tempLatitude = rmcLat;
           tempLongitude = rmcLon;
           latHem = rmcLatHem[0];
           lonHem = rmcLonHem[0];
           if(latHem=='S') tempLatitude = -tempLatitude;
           if(lonHem=='W') tempLongitude = -tempLongitude;
           latitude = convertDegMinToDecDeg(tempLatitude);
           longitude = convertDegMinToDecDeg(tempLongitude);
           goodGPS = 1;
           if(printDiags) {
            Serial.print("Lt:");
            Serial.print(latitude);
            Serial.print(" Ln:");
            Serial.println(longitude);
           }
        }
      }
    }
    // start new message here
    streamPos = 0;
  }
  gpsStream[streamPos] = incomingByte;
  streamPos++;
  if(streamPos >= maxChar) streamPos = 0;
}


void gpsSleep(){
//  Serial2.println("$PMTK161,0*28");
//  Serial2.flush();
}

void gpsHibernate(){
//  Serial2.println("$PMTK225,4*2F");
//  Serial2.flush();
}

void gpsWake(){
//  Serial2.println(".");
//  Serial2.flush();
}

void gpsSpewOff(){
  //Serial2.println(PMTK_SET_NMEA_OUTPUT_OFF);
}

void gpsSpewOn(){
 // Serial2.println(PMTK_SET_NMEA_OUTPUT_RMCONLY);
}

void waitForGPS(){
  for(int n=0; n<100; n++){
    delay(20);
    while (Serial2.available() > 0) {    
        byte incomingByte = Serial2.read();
        Serial.write(incomingByte);
    }
  }
}

//int gpsDumpLogger(){
//  // open file for storing data; append
//  endGpsLog = 0;
//   Serial2.println(PMTK_LOCUS_DUMP);
//   int dumping = 1;
//   while(endGpsLog==0){
//       while (Serial2.available() > 0) {    
//        byte incomingByte = Serial2.read();
//        gps(incomingByte);
//        Serial.write(incomingByte);
//       }
//    if(gpsTimeout >= gpsTimeOutThreshold) return 0;
//   }
//   return 1;
//}

double convertDegMinToDecDeg(float degMin) {
  double min = 0.0;
  double decDeg = 0.0;
 
  //get the minutes, fmod() requires double
  min = fmod((double)degMin, 100.0);
 
  //rebuild coordinates in decimal degrees
  degMin = (int) ( degMin / 100 );
  decDeg = degMin + ( min / 60 );
 
  return decDeg;
}


void gpsGetTimeLatLon(){
    // get GPS
  int incomingByte;
  long gpsTimeOutStart = millis();

  goodGPS = 0;
  gpsInit();
  
  // can't display GPS data here, because display slows things down too much
  while((!goodGPS) & (millis()-gpsTimeOutStart<gpsTimeOutThreshold)){
    while (Serial2.available() > 0) {    
        incomingByte = Serial2.read();
        Serial.write(incomingByte);
        gps(incomingByte);  // parse incoming GPS data
    }
  }

  Serial.print("GPS search time:");
  Serial.println(millis()-gpsTimeOutStart);
  
  Serial.print("Good GPS:");
  Serial.println(goodGPS);
}



// Set Nav Mode to Portable
static const uint8_t setNavPortable[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const int len_setNav = 42;

// Set Nav Mode to Pedestrian
static const uint8_t setNavPedestrian[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Set Nav Mode to Automotive
static const uint8_t setNavAutomotive[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Set Nav Mode to Sea
static const uint8_t setNavSea[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Set Nav Mode to Airborne <1G
static const uint8_t setNavAir[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Set NMEA Config
// Set trackFilt to 1 to ensure course (COG) is always output
// Set Main Talker ID to 'GP' instead of 'GN'
static const uint8_t setNMEA[] = {
  0xb5, 0x62, 0x06, 0x17, 0x14, 0x00, 0x20, 0x40, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const int len_setNMEA = 26;

// Set Low Power Mode
// Use this with caution
// Putting the M8 into low power mode before a fix has been established can cause it to reset after 10secs (searchPeriod)
static const uint8_t setLP[] = { 0xb5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x48, 0x01 };
static const int len_setLP = 8;

//// Set EXTINT to wake from sleep
//static const uint8_t setPMREQ[] = { 
//  0xB5, 0x62, 0x02, 0x41, 0x10, 0x00,
//  0x00, 0x00, 0x00, 0x00,
//  0x00, 0x00, 0x00, 0x00,
//  0x02, 0x00, 0x00, 0x00,
//  0x10, 0x00, 0x00, 0x00
//  };
//static const int len_setPMREQ = 22;

// UBX-RXM-PMREQ
// Go to sleep forevers
static const uint8_t setPMREQ[] = { 
  0xB5, 0x62, 0x02, 0x41, 0x08, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00,
  };
static const int len_setPMREQ = 14;

// UBX-CFG-PMS
// 0x00 -> Full Power
// 0x01 ->Balanced
// 0x02 -> Interval
// 0x03 -> Aggressive with 1 Hz (lowest power with continuous operation)
// 0x04 -> Aggressive with 2 Hz
// 0x05 -> Aggressive with 4 Hz
static const uint8_t setPMS[] = {
  0xB5, 0x62, 0x06, 0x86, 0x08, 0x00,
  0x00,
  0x03,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00
};
static const int len_setPMS = 14;

// UBX-MON-VER
static const uint8_t getVER[] = {
  0x5B, 0x62, 0x0A, 0x04, 0x00, 0x00
};
static const int len_getVER = 6;

// Set GNSS Config to GPS + Galileo + GLONASS + SBAS (Causes the M8 to restart!)
static const uint8_t setGNSS[] = {
  0xb5, 0x62, 0x06, 0x3e, 0x3c, 0x00,
  0x00, 0x20, 0x20, 0x07,
  0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01,
  0x02, 0x04, 0x08, 0x00, 0x01, 0x00, 0x01, 0x01,
  0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x03,
  0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x05,
  0x06, 0x08, 0x0e, 0x00, 0x01, 0x00, 0x01, 0x01 };
static const int len_setGNSS = 66;

static const uint8_t ubxVer[] = {
  0xB5, 0x62, 0x0A, 0x04
};
static const int len_ubxVer = 4;

// Send message in u-blox UBX format
// Calculates and appends the two checksum bytes
// Doesn't add the 0xb5 and 0x62 sync chars (these need to be included at the start of the message)
void sendUBX(const uint8_t *message, const int len) {
  int csum1 = 0; // Checksum bytes
  int csum2 = 0;
  Serial.print("Sending UBX packet: 0x");
  for (int i=0; i<len; i++) { // For each byte in the message
    Serial2.write(message[i]); // Write the byte

    if (message[i] < 16) {Serial.print("0");}
    Serial.print(message[i], HEX);
    Serial.print(", 0x");

    if (i >= 2) { // Don't include the sync chars in the checksum
      csum1 = csum1 + message[i]; // Update the checksum bytes
      csum2 = csum2 + csum1;
    }
  }
  csum1 = csum1 & 0xff; // Limit checksums to 8-bits
  csum2 = csum2 & 0xff;
  Serial2.write((uint8_t)csum1); // Send the checksum bytes
  Serial2.write((uint8_t)csum2);
  Serial2.flush();

  if (csum1 < 16) {Serial.print("0");}
  Serial.print((uint8_t)csum1, HEX);
  Serial.print(", 0x");
  if (csum2 < 16) {Serial.print("0");}
  Serial.println((uint8_t)csum2, HEX);
}


// checksum calculator https://nmeachecksum.eqth.net/
void gpsInit(){

  
  // disable all messages but RMC
  Serial2.println("$PUBX,40,GLL,0,0,0,0*5C"); // Disable GLL
  Serial2.flush();
  delay(10);
  Serial2.println("$PUBX,40,ZDA,0,0,0,0*44"); // Disable ZDA
  Serial2.flush();
  delay(10);
  Serial2.println("$PUBX,40,VTG,0,0,0,0*5E"); // Disable VTG
  Serial2.flush();
  delay(10);
  Serial2.println("$PUBX,40,GSV,0,0,0,0*59"); // Disable GSV
  Serial2.flush();
  delay(10);
  Serial2.println("$PUBX,40,GSA,0,0,0,0*4E"); // Disable GSA
  Serial2.flush();
  Serial2.println("$PUBX,40,GGA,0,0,0,0*5A"); // Disable GGA
  delay(10);
  Serial2.println("$PUBX,40,RMC,0,0,0,0*47"); // Disable RMC
  delay(10);
  Serial2.flush();
  delay(50);
  downloadSerialHex();
  
  Serial.println("Set Nav");
  sendUBX(setNavSea, len_setNav);
  Serial.println();
  delay(800);
  Serial.println("Response:");
  downloadSerialHex();


//  sendUBX(setLP, len_setLP);
//  Serial.println();
//  delay(800);
//  Serial.println("Response:");
//  downloadSerialHex();

//  sendUBX(setPMREQ, len_setPMREQ);
//  Serial.println("Set power management");
//  delay(800);
//  Serial.println("Response:");
//  downloadSerialHex();

  Serial2.println("$PUBX,40,RMC,0,1,0,0*46"); // Enable RMC
  delay(10);
  Serial2.flush();
}

void downloadSerialHex(){
    while (Serial2.available() > 0) {    
        uint8_t incomingByte = Serial2.read();
        //Serial.write(incomingByte);
        Serial.print(incomingByte, HEX);
        Serial.print(" ");
    }
    Serial.println();
}


void gpsLowPower(){
  //Serial.println("Set PMS 1 Hz");

  Serial.println("Set Nav");
  sendUBX(setNavSea, len_setNav);
  Serial.println();
  delay(800);
  Serial.println("Response:");
  downloadSerialHex();

  Serial.println("setPMS");
  sendUBX(setPMS, len_setPMS);
  //sendUBX(setPMREQ, len_setPMREQ);
  Serial.println();
  delay(800);
  Serial.println("Response:");
  downloadSerialHex();
}
