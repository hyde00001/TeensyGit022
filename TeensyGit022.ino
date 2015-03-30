
/*
*********1*********2*********3*********4*********5*********6*********7*********8
Teensy3.1 Based HAB Tracker

v022 - Move myGPSAirborne1Mode call to all GPS startups

*** Hardware Configuration ***

*** Teensy Power ***
During development, power is supplied by USB connection
During HAB use, power is supplied via battery pack connected to
  Vin on Teensy (3.7 - 5.5v tolerant)
  GND
  (yes these are on opposite sides of the board near USB connector)  
  
*** Hardware Power ***
Teensy provides: 
  3.3v (100mA max)
  GND
  (yes these are on opposite sides of the board near USB connector)

100uF (25V probbaly better if smaller) de-coupling capacitor across
3.3V and GND lines


*** USB Serial ***
Debugging messages are sent from Teensy3.1 to Serial
This can be read via Arduino development environment Serial Monitor on USB
 
*** GPS ***
Teensy3.1 communicates with uBLOX MAX-7 Q Breakout Board connected via Serial1
(see http://ava.upuaut.net/store/) 
  3.3v on Teensy3.1 connects to 3.3V on uBLOX MAX-7 Breakout Board
  GND on Teensy3.1 connects to GND on uBLOX MAX-7 Breakout Board
  Rx1 (Pin 0) on Teensy3.1 to 3VTX on uBLOX MAX-7 Breakout Board
  Tx1 (Pin 1) on Teensy3.1 to 3VRX on uBLOX MAX-7 Breakout Board
 
*** NTX2 434.650 MHz ***
Teensy3.1 communicates with NTX2 radio module via myRadioPin (defined below)
Then wired as described here: http://ukhas.org.uk/guides:linkingarduinotontx2

Pins:
1  RF Gnd - connect to aeriel coax braid / PCB ground plane
2  RF Out - connect to aeriel
3  RF Gnd - connect to aeriel coax braid / PCB ground plane

4  En     - Pull High (3V logic) to enable transmit
5  Vcc    - 2.9 - 15V DC
6  0V     - µControler GND
7  TXD    - transmit 3v logic trasmit. Rin 100K Ohmn


R1 + R2 (4K7) form a voltage divider between 3.3v & GND
The centre point is connected to TXD on NTX2
R3 (33K) connects from myRadioPin to TXD on NTX2 
 
EN & Vcc on NTX2 are connected to 3.3v
0V on NTX2 is connected to GND

100nF ceramic de-coupling capacitor across pins 5/6 (power supply)
 
For development, RF Out connected to Gnd strip (NOT Connected to GND!) on breadboard in lieu of an aerial 
 
*** Voltage Supply Check ***
R1 + R2 form a voltage divider between Vin & GNDA
3.3v 33K Centre 10K GNDA
The centre point is connected to myBatteryPin

*** DS18B20 ***
The DS18B20 "one wire bus" is connected to Teensy via my1WirePin (default 20)
Teensy communicates with a DS18B20 1-wire temperature probe using "parasite power" on the DQ line

1st sensor on bus: DS18B20 TO-92
Looking at flat face x3 pins left to right
GND is connected to GND on Teensy
DQ is connected to VCC via 4K7 & to my1WirePin
Vdd is connected to GND on Teensy

2nd sensor on bus: DS18B20 waterpooof probe
GND is black wire is connected to GND on Teensy
DQ is yellow wire is connected to DQ on 1st sensor
Vdd is red wire is connected to GND on Teensy

Index on bus depends on hardware encoded register (no software control)
Lowest hardware value = index 0
Highest hardware value = index 1





*** HabHub Configuration ***
Requires a HabHub.org payload configuration file

*** Radio & Telemetry ***
Primary
434.650 MHz
USB RTTY
370 Hz shift
ASCII-7
no parity
2 stop bits

*** Parser ***
Normal
Callsign:      HYDEST001
Checksum:      crc16-ccitt
sentence_id:   Integer
time:          Time
latitude:      Coordinate ddmm.mmmm
longitude:     Coordinate ddmm.mmmm
altitude:      Integer
gps_lock:      Integer
satellites:    Integer
battery:       Float
temp_internal: Float
temp_external: Float


*/

// Global variables

// *** HAB Call sign ***
String myCallSign = "$$HYDEST001"; // Change Me
char myCallSignPrefix [3] = {'$','$'}; // extra text at start of each RTTY message

// *** Debugging options ***
boolean myDebugAllOff = false;   //if true turns all debug options off
boolean myDebugAllOn = false;    //if true turns all debug options on - overides myDebugAllOff 

boolean myDebugShowFullMessage = false;
boolean myDebugShowTrimmedMessage = true;
boolean myDebugShowErrorChecksOnTrimmedMessage = false;
boolean myDebugShowExtractedGPSData = false;
boolean myDebugShowGPSMessageSubstrings = false;
boolean myDebugShowSensorValues = true;
boolean myDebugShowRTTY= true;
boolean myDebugSendRTTY = true;
boolean myDebugShowGPSStatus = true;


// *** GPS Unit Variables ***

// String for incoming GPS Message
String myGPSMessage;

// Parsed GPS Message Parameters
String myGPSTime;
String myGPSLat;
String myGPSLong;
String myGPSFix;
String myGPSSatellites;
String myGPSAltitude;

// Globals to count good/bad GPS Messages
// GPS Unit Hot/Warm/Cold Starts, GPS Fix status 
// GPS PowerMode & flight status
unsigned int myGoodGPSMessageCount = 0;
unsigned int myTotalBadGPSMessageCount = 0;
unsigned int myConsecutiveBadGPSMessageCount = 0;
unsigned int myGPSHotStartCount = 0;
unsigned int myGPSWarmStartCount = 0;
unsigned int myGPSColdStartCount = 0;
#define myBadMessageThreshold 20
#define myNoGPSFixThreshold 100
boolean myGPSCurrentlyHaveFix = false;
unsigned int myGPSNoFixCount = 0;
unsigned int myGPSLastFix = 0;

boolean myGPSFlightModeYesNo = false;
#define myGPSFlightModeAttemptsBeforeColdStart 10


boolean myGPSPowerSaveModeYesNo = false;
boolean myGPSMaxPowerModeYesNo = true;

// Calling myGPSGPGGAOnly () restricts GPS messages to GPGGA only
// Global to keep track of how many times we call it
unsigned int  myGPSGPGGAOnlyCount = 0;

// If we have GPS Fix we can enable PowerSave on GPS
// define here the minimum number of Satelittes before trying
#define myPowerSaveSatThreshold 6

// If we have GPS Fix we can enable Airborne mode on GPS
// define here the minimum number of Satelittes before trying
#define myAirborneModeSatThreshold 6



// *** RTTY Global variables ***
// Teensy pin for RTTY communication
#define myRadioPin 15

// RTTY Baud Rate
int myRTTYBaud = 1; //1=50 baud, 2=300 baud, 3=50 & 300 baud
int myBaud = 50;

// String/char for RTTY messages
String myRTTY;
char myRTTYchar[120];



// *** Tracker Status LED
// Teensy pin for LED communication
#define myLEDPin 13 // 13 has inbuilt LED
boolean myShowStatusLED = true;

// *** HAB Sensors 
// Battery Voltage
#define myBatteryPin 14
String myDefaultVolts = "NoVolts";
String myBatteryVolts = myDefaultVolts;

// Temperature Sensors
// DS18B20 Temperature Probe Libraries
#define my1WirePin 20

#include <DallasTemperature.h>
#include <OneWire.h>

// Tried to put next two lines into setup but then fails...
// Setup a oneWire instance to communicate with any OneWire devices
OneWire my1WireBus (my1WirePin);
// Pass our oneWire reference to Dallas Temperature library
DallasTemperature my1WireSensors (&my1WireBus);

String myTemp0 = "NoTemp0";
String myTemp1 = "NoTemp1";



void setup()
{
  delay (4000);  // let GPS unit wake itself up on powerup
  
  if (myDebugAllOff) { // turn all debug activity off
    myDebugShowFullMessage = false;
    myDebugShowTrimmedMessage = false;
    myDebugShowErrorChecksOnTrimmedMessage = false;
    myDebugShowExtractedGPSData = false;
    myDebugShowGPSMessageSubstrings = false;
    myDebugShowSensorValues = false;
    myDebugShowRTTY= false;
    myDebugSendRTTY = true;
    myDebugShowGPSStatus = false;
  }
  
  
  if (!myDebugAllOff) { // then we will need USB/Serial communication
    Serial.begin(115200);

    Serial.println("Starting Teensy HAB Tracker v020"); 
    Serial.println("By Steve Hyde");
    Serial.println("Adapted From Code Found At: http://ukhas.org.uk/guides:ublox6 & http://ukhas.org.uk/guides:linkingarduinotontx2");
    Serial.println();
    Serial.println();
    Serial.println();
  }
  
  //Set FlightMode & MaxPowerMode
  mySetFlightAndPowerOnRestart();  
  
  // On powerup the uBlox sends $GPRMC $GPVTG $GPGGA $GPGSA $GPGSV $GPGLL $GPTXT 
  // Turn off all messages except for GPGGA
  myGPSGPGGAOnly();
  myGPSGPGGAOnly(); // send it twice to bwe sure as once doesn't allways get it done

  //Enable Pins
  pinMode(myRadioPin,OUTPUT);
  pinMode(myLEDPin,OUTPUT);
  pinMode(myBatteryPin,INPUT);
    
  //initialise 1 wire bus
  my1WireSensors.begin(); 
  
  //Flash myStatusLED - once if we have GPS Fix, twice if not
  myStatusLED ();

}

void loop()
{
  // Read GPS unit message for 1.5 seconds and put into myGPSMessage
  unsigned long myTimeInMilliSeconds;

  myGPSMessage = "";
  boolean myError = false;

  // For 1.5 second we read serial data from the GPS unit and append it to myGPSMessage
  // nb initially used one second read but this occasionaly gave corrupt GPS messages
  Serial1.begin(9600);
  for (myTimeInMilliSeconds = millis(); millis() - myTimeInMilliSeconds < 1500;)
  { 
    while (Serial1.available())
    {
      // Read a char from Serial1 (the GPS Unit)
      char c = Serial1.read();

      //Serial.print(c); // uncomment this line if you want to see the GPS data flowing

      //Add the char to myGPSMessage 
      myGPSMessage += c;
    }
  }
  Serial1.end();

  // Process myGPSMessage return an error if the GPS message is bad 
  myError = myProcessGPSMessage();

  if (myError)
  {
    if (myDebugShowErrorChecksOnTrimmedMessage)
    {
      Serial.println(F("There Was An Error"));
    }
    // Increment the total and consecutive message counters
    myConsecutiveBadGPSMessageCount++;
    myTotalBadGPSMessageCount++;

    if (myConsecutiveBadGPSMessageCount >= myBadMessageThreshold)  
    {    
      // Lots of errors ( more than myBadMessageThreshold in a row) so restart the GPS unit      
      myGPSHotStart();
      mySetFlightAndPowerOnRestart();

      // Supress the GPS messages we don't want 
      myGPSGPGGAOnly();
      myGPSGPGGAOnly();

      // Reset the consecutive bad message counter
      myConsecutiveBadGPSMessageCount = 0;
    }
    else
    {
      delay (100); // Perhaps if we wait a bit the GPS unit will get its act together
    }

  }
  else
  {
    myGoodGPSMessageCount++;
    if (myDebugShowErrorChecksOnTrimmedMessage)
    {
      Serial.println(F("There Wasn't An Error"));
    }

    // decrement myConsecutiveBadGPSMessageCount
    if (myConsecutiveBadGPSMessageCount >= 1)
    {
      myConsecutiveBadGPSMessageCount--;
    }

    // Extract the GPGGA Message elements
    myExtractGPSMessageStrings();

    //Check GPSFix
    if (myGPSFix == "NoGPSFix")
    {
      // We have no GPS Fix
      if (myDebugShowErrorChecksOnTrimmedMessage){
        Serial.println(F("No GPSFix"));
      }
      myGPSCurrentlyHaveFix = false;

      // Assume GPS has restarted and lost GPSPowerSaveMode and/or GPSAirborne1Mode if it ever had it
      myGPSPowerSaveModeYesNo = false;
      myGPSFlightModeYesNo = false;

      // Start counting how long we have had no GPS Fix
      myGPSNoFixCount++;

      // If the GPSNoFixCount exceeds a global myNoGPSFixThreshold then we should restart the GPS Unit and see if that sort things out
      if (myGPSNoFixCount >= (myNoGPSFixThreshold))
      {

        // Typically the first data we get during GPS acqusition is GPS Time
        // Often we get time but no other data if we have weak satellite signal
        // If there is no GPS Time then a cold start is probbaly warranted
        // If there is GPS Time then perform a less destructive warm start (keeps any satellite data already acquired)
        if (myGPSTime == "No:GPS:Time")
        {
          // Perform cold start and then supress GPS Message types we don't want
          myGPSColdStart();
          mySetFlightAndPowerOnRestart();
          myGPSGPGGAOnly();
          myGPSGPGGAOnly();

          delay (5000);
          myGPSNoFixCount = 0;
        }
        else
        {
          // Perform warm start and then supress GPS Message types we don't want
          myGPSWarmStart();
          mySetFlightAndPowerOnRestart();
          myGPSGPGGAOnly();
          myGPSGPGGAOnly();

          delay (5000);
          myGPSNoFixCount = 0;
        }
      }

    }
    else
    {
      // We have GPS Fix 
      if (myDebugShowErrorChecksOnTrimmedMessage) {
        Serial.println(F("GPSFix"));
      }
      myGPSCurrentlyHaveFix = true;

      // Reset the GPSNoFix counter and start counting how long we have had GPS Fix
      myGPSNoFixCount = 0;
      myGPSLastFix = myGoodGPSMessageCount;

      // If the GPS Unit deosn't have PowerSave mode AND we have myPowerSaveSatThreshold satelittes then try and set it
      if (!myGPSPowerSaveModeYesNo && myGPSSatellites.toInt() >= myPowerSaveSatThreshold)
      {      
        myGPSPowerSaveMode();
        delay (1000); // HAS Wiki suggests a 1 sec delay before we do anything else 
      }

      // If the GPS Unit deosn't have Flight mode AND we have myAirborneModeSatThreshold satelittes then try and set it      
      if (!myGPSFlightModeYesNo && myGPSSatellites.toInt() >= myAirborneModeSatThreshold)
      {
        myGPSFlightMode();
        delay (1000); // HAS Wiki suggests a 1 sec delay before we do anything else
      }

    }

    // Now Read Sensors etc
    myReadSensors();
    
    // Show GPS Fix Status LED
    myStatusLED (); 

    // Now compose RTTY Message
    myComposeRTTY();


  }

  if (myDebugShowGPSStatus) {
    Serial.println(F(""));
    Serial.print(F("Good GPS Message Count: "));
    Serial.print(myGoodGPSMessageCount);
    Serial.print(F("   "));
    Serial.print(F("Total Bad GPS Message Count: "));
    Serial.print(myTotalBadGPSMessageCount);
    Serial.print(F("   "));
    Serial.print(F("Current Consecutive Bad GPS Message Count: "));
    Serial.print(myConsecutiveBadGPSMessageCount);
    Serial.println(F("   "));
    Serial.print(F("GPS HotStart Count: "));
    Serial.print(myGPSHotStartCount);
    Serial.print(F("   "));
    Serial.print(F("GPS WarmStart Count: "));
    Serial.print(myGPSWarmStartCount);
    Serial.print(F("   "));
    Serial.print(F("GPS ColdStart Count: "));
    Serial.print(myGPSColdStartCount);
    Serial.println(F("   "));
    Serial.print(F("GPS Currently Have Fix: "));
    Serial.print(myGPSCurrentlyHaveFix);
    Serial.print(F("   "));
    Serial.print(F("GPS LastFix: "));
    Serial.print(myGPSLastFix);
    Serial.print(F("   "));
    Serial.print(F("GPS NoFix Count: "));
    Serial.print(myGPSNoFixCount);
    Serial.println(F("   "));
    Serial.print(F("GPS PowerSave Mode: "));
    Serial.print(myGPSPowerSaveModeYesNo);
    Serial.print(F("   "));
    Serial.print(F("GPS Flight Mode: "));
    Serial.print(myGPSFlightModeYesNo);
    Serial.println(F("   "));


    //Print an empty line for a new cycle
    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F(""));
    Serial.println(F(""));
  }
  
   
} 




















