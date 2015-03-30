void myReadSensors ()
{
  // Battery Voltage
  
  // Voltage divider installed between 3.3v rail and myBatteryPin
  // R1 33K
  // R2 10K
  // Should reduce the applied voltage - suitable for USB 5v or 3xAA 4.8v
  
  float R1 = 33000;
  float R2 = 10000; 
  
  int myVoltRead = 0;
  
  // Read battery voltage on pin myBatteryPin
  // Pre-read to stabalise reading
  myVoltRead = analogRead(myBatteryPin);
  delay (10);
  // Actual read
  myVoltRead = analogRead(myBatteryPin);
  
  // Convert analog read vale (0-1023) to 0-3.3v 
  // float myVoltFloat = ((myVoltRead / 1023.0) / ((R2) / (R1 + R2));
  float myVoltFloat = (myVoltRead * 3.3 / 1024.0) / ((R2) / (R1+R2));
  myVoltFloat = myVoltFloat + 0.04; // seems to be 0.04v out so add it back...
  
  // convert float to String
  // dtostrf takes a double/float, width, precission, char and returns pointer to the char
  int myWidth = 1;  // minimum number of characters including possible - or . 
  int myPrec = 2;   // number of digits after the decimal sign
  char buffer[32];  // char buffer for storage of conversion
  // myWidth = 1, myPrec = 2 will give a number X.XX as would 3,2
  
  myBatteryVolts = dtostrf(myVoltFloat, myWidth, myPrec, buffer);
  
  
  // DS18B20 Temperature Probe - code assumes two attached 
  // wonder what happens if one/more fails - should test!
  my1WireSensors.requestTemperatures(); // Send the command to get temperatures
  float myTemp0Celcius = my1WireSensors.getTempCByIndex(0); // index 0 "furthest" away from Teensy
  float myTemp1Celcius = my1WireSensors.getTempCByIndex(1); // index 1 nearer Teesny
  
  // convert floats to String. Recylce char buffer from 
  myTemp0 = dtostrf(myTemp0Celcius, 1, 2, buffer);
  myTemp1 = dtostrf(myTemp1Celcius, 1, 2, buffer);
  
  
  if (myDebugShowSensorValues)
  {
    Serial.print(F("Battery Analogue Read: "));
    Serial.print(myVoltRead);
    Serial.print(F(" "));
    Serial.print(F("Converted Battery Voltage: "));
    Serial.print(myBatteryVolts);
    Serial.println(F(" "));
    Serial.print(F("Temp 0: "));
    Serial.println(myTemp0);
    Serial.print(F("Temp 1: "));
    Serial.println(myTemp1);  
  }
}



void myStatusLED () // Flash LED - once for GPS Fix, twice for not
{
 if (myShowStatusLED) {
    digitalWrite(myLEDPin, HIGH);
    delay(200);
    digitalWrite(myLEDPin, LOW);
    
    if (!myGPSCurrentlyHaveFix) {
      delay (200);
      digitalWrite(myLEDPin, HIGH);
      delay(200);
      digitalWrite(myLEDPin, LOW);
    }
  }
}
