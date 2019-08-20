String dataPacket;

void makeDataPacket(){
  //sprintf(dateTime,"%04d%02d%02dT%02d%02d%02d", year(packetTime), month(packetTime), day(packetTime), hour(packetTime), minute(packetTime), second(packetTime));
  
  // 31 Bytes:   dateTime,duration,lat,long
//  dataPacket = "";
//  dataPacket += packetTime;
//  dataPacket += ";";
  dataPacket = "";
  dataPacket += "RB0012628";  // deliver to this Rockblock
  dataPacket += String(latitude, 4);
  dataPacket += ";";
  dataPacket += String(longitude, 4);
  SerialUSB.println(dataPacket);
}

int sendDataPacket(){
  int err = 0;
  //int err = modem.sendSBDText("20180817T140000;26.4321;-82.3476;g:12;nh:[70,10,1,4]");
    err = modem.sendSBDText(&dataPacket[0]);
    if (err != ISBD_SUCCESS)
    {
      SerialUSB.print("sendSBDText error: ");
      SerialUSB.println(err);
  
      if (err == ISBD_SENDRECEIVE_TIMEOUT){
        SerialUSB.println("Send timeout");
      }
    }
  return err;
}
