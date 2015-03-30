void myComposeRTTY()
{
  myRTTY = "";
  
  myRTTY += myCallSign;
  myRTTY += ",";

  myRTTY += myGoodGPSMessageCount;
  myRTTY += ",";

  myRTTY += myGPSTime;
  myRTTY += ",";

  myRTTY += myGPSLat;
  myRTTY += ",";

  myRTTY += myGPSLong;
  myRTTY += ","; 

  myRTTY += myGPSAltitude;
  myRTTY += ",";

  myRTTY += myGPSFix;
  myRTTY += ",";

  myRTTY += myGPSSatellites;
  myRTTY += ",";

  myRTTY += myBatteryVolts;
  myRTTY += ",";
  
  myRTTY += myTemp0;
  myRTTY += ",";
  
  myRTTY += myTemp1;
  

  myRTTY.toCharArray (myRTTYchar,120);

  // Calculates the checksum for this datastring
  unsigned int CHECKSUM = gps_CRC16_checksum(myRTTYchar);  // Calculates the checksum for this datastring
  char checksum_str[6]; 
  sprintf(checksum_str, "*%04X\n", CHECKSUM);
  strcat(myRTTYchar,checksum_str);


  if (myDebugShowRTTY)
  {
    Serial.print(F("Composed RTTY String:"));
    Serial.println(myRTTYchar);
  }

  //Send the RTTY over the radio!

  if (myRTTYBaud == 1 || myRTTYBaud == 3)
  {
    myBaud = 50;
  }
  else
  {
    myBaud = 300;
  }

  // Now compose RTTY Message
  if (myDebugSendRTTY)
  {
    if (myDebugShowRTTY) {
      Serial.print(F("Sending RTTY String @ "));
      Serial.print(myBaud);
      Serial.print(F(" Baud "));
      Serial.println(F(""));
    }
    rtty_txstring (myCallSignPrefix);
    rtty_txstring (myRTTYchar);    
  }

  if (myRTTYBaud ==3) // send it twice, second time at 300
  {
    myBaud = 300;
    // Now compose RTTY Message
    if (myDebugSendRTTY)
    {
      if (myDebugShowRTTY) {
        Serial.print(F("Sending RTTY String @ "));
        Serial.print(myBaud);
        Serial.print(F(" Baud "));
        Serial.println(F(""));
      }
      rtty_txstring (myCallSignPrefix);
      rtty_txstring (myRTTYchar);     
    }
  } 

}





void rtty_txstring (char * string)
{

  /* Simple function to sent a char at a time to 
   	** rtty_txbyte function. 
   	** NB Each char is one byte (8 Bits)
   	*/

  char c;

  c = *string++;

  while ( c != '\0')
  {
    rtty_txbyte (c);
    c = *string++;
    if (myDebugShowRTTY) {
      Serial.print(F("."));
    }
  }
}


void rtty_txbyte (char c)
{
  /* Simple function to sent each bit of a char to 
   	** rtty_txbit function. 
   	** NB The bits are sent Least Significant Bit first
   	**
   	** All chars should be preceded with a 0 and 
   	** proceded with a 1. 0 = Start bit; 1 = Stop bit
   	**
   	*/

  int i;

  rtty_txbit (0); // Start bit

  // Send bits for for char LSB first	

  for (i=0;i<7;i++) // Change this here 7 or 8 for ASCII-7 / ASCII-8
  {
    if (c & 1) rtty_txbit(1); 

    else rtty_txbit(0);	

    c = c >> 1;

  }

  rtty_txbit (1); // Stop bit
  rtty_txbit (1); // Stop bit
}

void rtty_txbit (int bit)
{
  if (bit)
  {
    // high
    digitalWrite(myRadioPin, HIGH);
  }
  else
  {
    // low
    digitalWrite(myRadioPin, LOW);

  }

  if (myBaud == 50)
  {
    delayMicroseconds(10000); // For 50 Baud uncomment this and the line below. 
    delayMicroseconds(10150); // You can't do 20150 it just doesn't work as the
    // largest value that will produce an accurate delay is 16383
    // See : http://arduino.cc/en/Reference/DelayMicroseconds
  }

  if (myBaud == 300)
  {
    delayMicroseconds(3370); // 300 baud
  }


}

uint16_t gps_CRC16_checksum (char *string)
{
  size_t i;
  uint16_t crc;
  uint8_t c;

  crc = 0xFFFF;

  // Calculate checksum ignoring the first two $s
  for (i = 2; i < strlen(string); i++)
  {
    c = string[i];
    crc = _crc_xmodem_update (crc, c);
  }

  return crc;
} 

uint16_t _crc_xmodem_update(uint16_t crc, uint8_t data)
{
  unsigned int i;

  crc = crc ^ ((uint16_t)data << 8);
  for (i=0; i<8; i++) {
    if (crc & 0x8000) {
      crc = (crc << 1) ^ 0x1021;
    } 
    else {
      crc <<= 1;
    }
  }
  return crc;
}

