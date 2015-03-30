boolean  myProcessGPSMessage()
{
  boolean myError = false; // assume no error
  
  
  // Find first $ in myGPSMessage
  int myFirstDollar = myGPSMessage.indexOf('$');
  
  // If no $ found then myFirstDollar will contain -1, myGPSMessage is no good so return with an error
  if (myFirstDollar == -1)
  {
    if (myDebugShowTrimmedMessage)
    {
      Serial.println(F(""));
      Serial.println(F("Didn't Find a $"));
    }   
    myError = true; // we have an error
  }
  else 
  {
    // we found a $. Trim myGPSMessage such that it starts with the first $
    myGPSMessage = myGPSMessage.substring (myFirstDollar);
    
    // recalculate position of first $, should be 0
    myFirstDollar = myGPSMessage.indexOf('$');
    
    if (myFirstDollar != 0)
    {
      myError = true; // we have an error
    }
  }
  
  // If we couldn't find a $ or trim myGPSMessage to start with it then return with an error
  if (myError)
  {
    return myError;
  }
  
  // Find first * in myGPSMessage
  int myFirstAsterix = myGPSMessage.indexOf('*');
  
  // If no * found then myFirstAsterix will contain -1
  // myGPSMessage is no good so return with an error
  if (myFirstAsterix ==-1)
  { 
    if (myDebugShowTrimmedMessage)
    {
      Serial.println(F(""));
      Serial.println(F("Didn't Find a *"));
    }   
    myError = true; // we have an error
  } 
  if (myError)
  {
    return myError;
  }
  
  int myMessageLength = myGPSMessage.length();
  // Make Sure the message is long enough to allow us to trim using myFirstAsterix+3  
  if (myMessageLength <= myFirstAsterix+3)
  {
    if (myDebugShowTrimmedMessage)
    {
      Serial.println(F(""));
      Serial.println(F("Message Not Long Enough"));
    }   
    myError = true; // we have an error
  } 
  if (myError)
  {
    return myError;
  }
  
  //Extract the GPS message without any bytes after the checksum - newline & carriage return are expected 
  myGPSMessage = myGPSMessage.substring (myFirstDollar, myFirstAsterix+3);
  
  if (myDebugShowTrimmedMessage)
  {
    Serial.print(F("Here is the trimmed message: "));
    Serial.println(myGPSMessage);
  }
  
  // Recalculate myGPSMessageLength
  myMessageLength = myGPSMessage.length();
  
  //Using the length create two new strings from myGPSMessage
  //myGPSMessageChecksum contains just the checksum
  //myChecksumFreeGPSMesage contains everthing between the $ and *
  String myGPSMessageChecksum = myGPSMessage.substring (myMessageLength-2, myMessageLength);
  String myChecksumFreeGPSMesage = myGPSMessage.substring (1, myMessageLength-2);
  
  //We need to convert myChecksumFreeGPSMesage to a char myGPSMessageToConfirm
  myMessageLength = myChecksumFreeGPSMesage.length();
  char myGPSMessageToConfirm [myMessageLength];
  myChecksumFreeGPSMesage.toCharArray (myGPSMessageToConfirm,myMessageLength);
    
  //Calculate the NMEA checksum for myGPSMessageToConfirm 
  int myChecksum=0;
 
  for(int i=0; i<myMessageLength; i++)
  {
    myChecksum ^= myGPSMessageToConfirm[i];
  }
  
  //Hex alphabetical characters come out lower case so convert them to NMEA upper case
  String myChecksumString = String(myChecksum, HEX);
  myChecksumString.toUpperCase();
  
  //Now test the checksum
  if (myGPSMessageChecksum != myChecksumString)
  {
    if (myDebugShowErrorChecksOnTrimmedMessage)
    {
      Serial.print (F("myGPSMessageChecksum: "));
      Serial.println(myGPSMessageChecksum);
      
      Serial.print (F("myChecksumString: "));
      Serial.println(myChecksumString);
      
      Serial.println(F("GPGGA Message Checksum Failed: Message Invalid"));
    }
    myError = true;
  }
  else
  {
    if (myDebugShowErrorChecksOnTrimmedMessage)
    {
      Serial.print (F("myGPSMessageChecksum: "));
      Serial.println(myGPSMessageChecksum);
      
      Serial.print (F("myChecksumString: "));
      Serial.println(myChecksumString);
      
      Serial.println(F("GPGGA Message Checksum Confirmed: Message Valid"));
    }
  }
  
  //Confirm myGPSMessage starts with $GPGGA  
  if (myGPSMessage.substring (0,6) != "$GPGGA")
  {
    if (myDebugShowErrorChecksOnTrimmedMessage)
    {
      Serial.println(F("Message Doesn't begin with $GPGGA"));
    }
    myError = true;
  }
  else
  {
    if (myDebugShowErrorChecksOnTrimmedMessage)
    {
      Serial.println(F("Message Begins with $GPGGA"));
    }
  }
  return myError;
}

/* Structure of a $GPGGA message
$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
includes 14 commas
$GPGGA,UTC,Lat,N/S,Long,E/W,GPSFix,Satellites,Horizontal Dilution,Altitude,M,Sealevel,M,,*66
      1   2   3   4    5   6      7          8                   9       10 11      12 13 14
Where:
     GGA          Global Positioning System Fix Data
     123519       Fix taken at 12:35:19 UTC
     4807.038,N   Latitude 48 deg 07.038' N
     01131.000,E  Longitude 11 deg 31.000' E
     1            Fix quality: 0 = invalid
                               1 = GPS fix (SPS)
                               2 = DGPS fix
                               3 = PPS fix
			       4 = Real Time Kinematic
			       5 = Float RTK
                               6 = estimated (dead reckoning) (2.3 feature)
			       7 = Manual input mode
			       8 = Simulation mode
     08           Number of satellites being tracked
     0.9          Horizontal dilution of position
     545.4,M      Altitude, Meters, above mean sea level
     46.9,M       Height of geoid (mean sea level) above WGS84
                      ellipsoid
     (empty field) time in seconds since last DGPS update
     (empty field) DGPS station ID number
     *47          the checksum data, always begins with *
*/


//Process myGPSMessage to extract the GPS data
void  myExtractGPSMessageStrings()
{
  String myGPSMessageSubStrings [20];
  int myGPSMessageLength = myGPSMessage.length();
  
  if (myDebugShowGPSMessageSubstrings)
  {
    Serial.print(F("myGPSMessage Length is: "));
    Serial.println(myGPSMessageLength);
  }
  
  int myCommaCounter = 1;
  int myLastComma = -1;
  int myNewComma = 0;
  
  //Step through myGPSMessage and find commas then use these to break the message up into myGPSMessageSubStrings [array]
  for (int i=0; i<=myGPSMessageLength; i++)
  {
    if (myGPSMessage.substring(i,i+1)==",")
    {
      myNewComma = i;

      myGPSMessageSubStrings[myCommaCounter]= myGPSMessage.substring(myLastComma+1,myNewComma);
      
      if (myDebugShowGPSMessageSubstrings)
      {
        Serial.println(myGPSMessageSubStrings [myCommaCounter]);
      }
      
      //Update myLastComma and myCommaCounter as we leave the if 
      myLastComma = myNewComma;
      myCommaCounter ++;
    }
  }
  
  //No Comma at the end. So have to find the last part of message here 
  myGPSMessageSubStrings[myCommaCounter]= myGPSMessage.substring(myLastComma+1,myGPSMessageLength);
  
  if (myDebugShowGPSMessageSubstrings)
  {
    Serial.println(myGPSMessageSubStrings [myCommaCounter]);
  
    Serial.print(F("Final Comma Count: "));
    Serial.println(myCommaCounter);
  }
  
  //Now parse myGPSMessageSubStrings[] into GPS global variables
  
  String myTempString;
  String myTempStringModifier;
  
  //Encode myGPSTime
  myGPSTime = "";
  
  //If myGPSMessageSubStrings [2] contains time data then process it into myGPSTime
  myTempString = myGPSMessageSubStrings [2];
  
  //Valid GPS time is nine digits x2 for hours x2 min x2 sec PERIOD x2 for 1/100ths
  if (myTempString.length() != 9)
  {
    myGPSTime = "No:GPS:Time";
  }
  else
  {
    //Extract Hours from myGPSMessageSubStrings [2] and add to myGPSTime
    myGPSTime = myTempString.substring (0, 2);
    myGPSTime +=":";
    
    //Extract Minutes from myGPSMessage and add to myGPSTime
    myGPSTime += myTempString.substring (2, 4);
    myGPSTime +=":";
    
    //Extract Seconds from myGPSMessage and add to myGPSTime
    myGPSTime += myTempString.substring (4, 6);
  }
  
  
  //Reset myGPSLat
  myGPSLat = "";
  
  //If myGPSMessageSubStrings [4] contains N/S data then process myGPSMessageSubStrings [3] & [4] into myGPSLat
  myTempString = myGPSMessageSubStrings [3];
  myTempStringModifier = myGPSMessageSubStrings [4];
  
  if (myTempStringModifier != "N" && myTempStringModifier != "S")
  {
    myGPSLat = "NoGPSLat";
  }
  else
  {
    //Add "-" if in Southern hemisphere
    if (myTempStringModifier == "S")
    {
      myGPSLat = "-";
    }
    
    //Then add the Latitude
    myGPSLat += myTempString;
  }
  
  //Reset myGPSLong
  myGPSLong = "";
  
  //If myGPSMessageSubStrings [6] contains E/W data then process myGPSMessageSubStrings [5] & [6] into myGPSLat
  myTempString = myGPSMessageSubStrings [5];
  myTempStringModifier = myGPSMessageSubStrings [6];
  
  if (myTempStringModifier != "E" && myTempStringModifier != "W")
  {
    myGPSLong = "NoGPSLong";
  }
  else
  {
    //Add "-" if in Western hemisphere
    if (myTempStringModifier == "W")
    {
      myGPSLong = "-";
    }
    
    //Then add the Latitude
    myGPSLong += myTempString;
  }
  
  //Reset myGPSFix
  myGPSFix = "";
  
  //If myGPSMessageSubStrings [7] contains GPSFix data then process it into myGPSFix
  myTempString = myGPSMessageSubStrings [7];
  
  if (myTempString.length() != 1 || myTempString =="0")
  {
    myGPSFix = "NoGPSFix";
  }
  else
  {
    myGPSFix = myTempString;
  }
  
  
  //Reset myGPSSatellites
  myGPSSatellites = "";
  
  //If myGPSMessageSubStrings [8] contains satellites data then process it into myGPSSatellites
  myTempString = myGPSMessageSubStrings [8];
  
  if (myTempString.length() == 0)
  {
    myGPSSatellites = "NoGPSSatellites";
  }
  else
  {
    myGPSSatellites = myTempString;
  }
  
  //Reset myGPSAltitude
  myGPSAltitude = "";
  
  //If myGPSMessageSubStrings [11] contains M data then process myGPSMessageSubStrings [10] into myGPSAltitude
  myTempString = myGPSMessageSubStrings [10];
  myTempStringModifier = myGPSMessageSubStrings [11];
  
  if (myTempStringModifier != "M" || myTempString.length() == 0)
  {
    myGPSAltitude = "NoGPSAltitude";
  }
  else
  {
    //Truncate Altitude data to an integer
    int myFullStop = myTempString.indexOf (".");
    if (myFullStop != -1)
    {
      myGPSAltitude = myTempString.substring (0, myFullStop);
    }
    else
    {
      myGPSAltitude = myTempString;
    }
  }
  
  // Depending on global debug status pass the extracted data to USB Serial
  if (myDebugShowExtractedGPSData)
  {
    Serial.print(F("North South: "));
    Serial.println(myGPSMessageSubStrings[4]);
        
    Serial.print(F("East West: "));
    Serial.println(myGPSMessageSubStrings[6]); 
    
    Serial.print(F("Extracted GPS Time Is: "));
    Serial.println(myGPSTime);
  
    Serial.print(F("Extracted GPS Lat Is: "));
    Serial.println(myGPSLat);
    
    Serial.print(F("Extracted GPS Long Is: "));
    Serial.println(myGPSLong);
    
    Serial.print(F("Extracted GPS Fix Is: "));
    Serial.println(myGPSFix);
    
    Serial.print(F("Extracted GPS Satellites: "));
    Serial.println(myGPSSatellites);
    
    Serial.print(F("Extracted GPS Altitude: "));
    Serial.println(myGPSAltitude);
  }

}

