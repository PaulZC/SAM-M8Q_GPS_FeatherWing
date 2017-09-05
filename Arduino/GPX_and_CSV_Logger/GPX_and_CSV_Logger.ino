// Log GPS position to SD card in GPX track and CSV format

// This code is written for the Adalogger M0 Feather
// https://www.adafruit.com/products/2796
// and is based extensively on Adafruit's GPS_HardwareSerial_Parsing example
// https://github.com/adafruit/Adafruit_GPS/tree/master/examples/GPS_HardwareSerial_Parsing

// Works with both the Adafruit Ultimate GPS FeatherWing
// https://www.adafruit.com/product/3133
// and Paul's u-blox M8 FeatherWings
// https://github.com/PaulZC/SAM-M8Q_GPS_FeatherWing
// https://github.com/PaulZC/MAX-M8Q_GPS_FeatherWing

#define UBLOX // Comment this line out to select the Adafruit Ultimate GPS
//#define LOWPOWER // Comment this line out to disable u-blox M8 low power mode
//#define DEBUG // Comment this line out to disable extra serial debug messages
//#define GALILEO // Comment this line out to use the default GNSS: GPS + SBAS + QZSS + GLONASS

// Connect a normally-open push-to-close switch between swPin and GND.
// Press it to stop logging.
#define swPin 15 // Digital Pin 15 (0.2" away from the GND pin on the Adalogger)

// Include the Adafruit GPS Library
// https://github.com/adafruit/Adafruit_GPS
#include <Adafruit_GPS.h>
Adafruit_GPS GPS(&Serial1); // M0 hardware serial
     
// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO false
     
// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;

// SD card logging
#include <SPI.h>
#include <SD.h>
#define cardSelect 4
File gpx_dataFile;
File csv_dataFile;
char gpx_filename[] = "20000000/000000.gpx";
char csv_filename[] = "20000000/000000.csv";
char dirname[] = "20000000";
boolean fileReady = false;

// LEDs
#define RedLED 13
#define GreenLED 8

// Count number of valid fixes before starting to log (to avoid possible corrupt file names)
int valfix = 0;
#define maxvalfix 4 // Collect at least this many valid fixes before logging starts (must be >= 2)

// Track Point
long trkpt = 0;

#ifdef UBLOX // Definitions for u-blox M8 UBX-format (binary) messages

// Set Nav Mode to Portable
static const uint8_t setNavPortable[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10 };
static const int len_setNav = 44;

// Set Nav Mode to Pedestrian
static const uint8_t setNavPedestrian[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x76 };

// Set Nav Mode to Automotive
static const uint8_t setNavAutomotive[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x98 };

// Set Nav Mode to Sea
static const uint8_t setNavSea[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0xBA };

// Set Nav Mode to Airborne <1G
static const uint8_t setNavAir[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC };

// Set NMEA Config
// Set trackFilt to 1 to ensure course (COG) is always output
// Set Main Talker ID to 'GP' instead of 'GN'
static const uint8_t setNMEA[] = {
  0xb5, 0x62, 0x06, 0x17, 0x14, 0x00, 0x20, 0x40, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x96, 0xd9 };
static const int len_setNMEA = 28;

// Set Low Power Mode
// Use this with caution
// Putting the MAX-M8Q into low power mode before a fix has been established can cause it to reset after 10secs (searchPeriod)
static const uint8_t setLP[] = { 0xb5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x48, 0x01, 0x62, 0x12 };
static const int len_setLP = 10;

// Set GNSS Config to GPS + Galileo + GLONASS (no SBAS or QZSS) (Causes a reset of the M8!)
static const uint8_t setGNSS[] = {
  0xb5, 0x62, 0x06, 0x3e, 0x3c, 0x00, 0x00, 0x20, 0x20, 0x07,
  0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x02, 0x04, 0x08, 0x00, 0x01, 0x00, 0x01, 0x01,
  0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x03,
  0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x05,
  0x06, 0x08, 0x0e, 0x00, 0x01, 0x00, 0x01, 0x01,
  0x54, 0x1b };
static const int len_setGNSS = 68;

#endif

void setup()
{
  // initialize digital pins RedLED and GreenLED as outputs.
  pinMode(RedLED, OUTPUT); // Red LED
  pinMode(GreenLED, OUTPUT); // Green LED
  // flash red and green LEDs on reset
  for (int i=0; i <= 4; i++) {
    digitalWrite(RedLED, HIGH);
    delay(200);
    digitalWrite(RedLED, LOW);
    digitalWrite(GreenLED, HIGH);
    delay(200);
    digitalWrite(GreenLED, LOW);
  }

  // initialize swPin as an input for the stop switch
  pinMode(swPin, INPUT_PULLUP);

  //delay(10000); // Allow 10 sec for user to open serial monitor
  
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  Serial.begin(115200);
  Serial.println("Log GPS Position to SD card in GPX and CSV format");
  Serial.println("Green LED = GPS Fix");
  Serial.println("Red LED Flash = SD Write");
  Serial.println("Continuous Red indicates a problem!");

  Serial.println("Initializing GPS...");

#ifdef UBLOX

  // u-blox M8 Init
  // 9600 is the default baud rate for u-blox M8
  GPS.begin(9600);
  // Disable all messages except GGA and RMC
  GPS.sendCommand("$PUBX,40,GLL,0,0,0,0*5C"); // Disable GLL
  delay(100);
  GPS.sendCommand("$PUBX,40,ZDA,0,0,0,0*44"); // Disable ZDA
  delay(100);
  GPS.sendCommand("$PUBX,40,VTG,0,0,0,0*5E"); // Disable VTG
  delay(100);
  GPS.sendCommand("$PUBX,40,GSV,0,0,0,0*59"); // Disable GSV
  delay(100);
  GPS.sendCommand("$PUBX,40,GSA,0,0,0,0*4E"); // Disable GSA
  delay(100);
  GPS.sendCommand("$PUBX,40,GGA,0,5,0,0*5F");  // Set GPGGA message rate to 5 (secs) - comment out for 1 sec logging
  delay(100);
  GPS.sendCommand("$PUBX,40,RMC,0,5,0,0*42");  // Set GPRMC message rate to 5 (secs) - comment out for 1 sec logging
  delay(100);
  Serial1.write(setNavPortable, len_setNav); // Set Portable Navigation Mode
  //Serial1.write(setNavPedestrian, len_setNav); // Set Pedestrian Navigation Mode
  //Serial1.write(setNavAutomotive, len_setNav); // Set Automotive Navigation Mode
  //Serial1.write(setNavSea, len_setNav); // Set Sea Navigation Mode
  //Serial1.write(setNavAir, len_setNav); // Set Airborne <1G Navigation Mode
  delay(100);
  Serial1.write(setNMEA, len_setNMEA); // Set NMEA configuration (use GP prefix instead of GN otherwise parsing will fail)
#ifdef GALILEO
  delay(100);
  Serial1.write(setGNSS, len_setGNSS); // Set GNSS - causes a reset of the M8!
#endif
  delay(1100);

#else

  // Adafruit Ultimate GPS Init
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  // turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Set the update rate
  //GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ); // 5 sec update rate
  // Disable updates on antenna status
  GPS.sendCommand(PGCMD_NOANTENNA);

#endif

  Serial.println("GPS initialized!");

  // flash the red LED during SD initialisation
  digitalWrite(RedLED, HIGH);

  // Initialise SD card
  Serial.println("Initializing SD card...");
  // See if the SD card is present and can be initialized:
  if (!SD.begin(cardSelect)) {
    Serial.println("Panic!! SD Card Init failed, or not present!");
    // don't do anything more:
    while(1);
  }
  Serial.println("SD Card initialized!");

  // turn red LED off
  digitalWrite(RedLED, LOW);

  Serial.println("Waiting for GPS fix...");
}
     
void loop() // run over and over again
{
  // Check if stop switch has been pressed
  if (digitalRead(swPin) == LOW) {
    Serial.println("Stop Switch Pressed! Waiting for reset...");
    digitalWrite(RedLED, HIGH); // turn the red led on
    while(1); // wait for reset
  }
  
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c) Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // Let the arrival of an new NMEA message be our second marker
    char *lastNMEA = GPS.lastNMEA(); // Make a copy of the string in case a new one arrives during parse (possibly redundant?)
    if (!GPS.parse(lastNMEA)) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another

    // Choose whether to log on the arrival of the GGA or the RMC message
    // (choose whichever one is sent out second by the GPS)
#ifdef UBLOX
    if (strstr(lastNMEA,"$GPGGA")) { // u-blox M8: wait for a new GGA message
#else
    if (strstr(lastNMEA,"$GPRMC")) { // Ultimate GPS: wait for a new RMC message
#endif

#ifdef DEBUG
      Serial.print("\nTime: ");
      Serial.print(GPS.hour, DEC); Serial.print(':');
      Serial.print(GPS.minute, DEC); Serial.print(':');
      Serial.print(GPS.seconds, DEC); Serial.print('.');
      Serial.println(GPS.milliseconds);
      Serial.print("Date: ");
      Serial.print(GPS.day, DEC); Serial.print('/');
      Serial.print(GPS.month, DEC); Serial.print("/20");
      Serial.println(GPS.year, DEC);
      Serial.print("Fix: "); Serial.print((int)GPS.fix);
      Serial.print(" Quality: "); Serial.println((int)GPS.fixquality);
      if (GPS.fix) {
        Serial.print("Location: ");
        Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
        Serial.print(", ");
        Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
        Serial.print("Speed (knots): "); Serial.println(GPS.speed);
        Serial.print("Angle: "); Serial.println(GPS.angle);
        Serial.print("Altitude: "); Serial.println(GPS.altitude);
        Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
        Serial.print("HDOP: "); Serial.println(GPS.HDOP);
      }
#endif

      // read battery voltage
      float vbat = analogRead(A7) * (2.0 * 3.3 / 1023.0);
#ifdef DEBUG
      Serial.print("Battery(V): ");
      Serial.println(vbat, 2);
#endif
    
      // turn green LED on to indicate GPS fix
      if (GPS.fix) {
        digitalWrite(GreenLED, HIGH);
        // increment valfix and cap at maxvalfix
        // don't do anything fancy in terms of decrementing valfix as we want to keep logging even if the fix is lost
        valfix += 1;
        if (valfix > maxvalfix) valfix = maxvalfix;
      }
      else {
        digitalWrite(GreenLED, LOW);
      }

#ifdef UBLOX
#ifdef LOWPOWER
      // Check if Low Power Mode can be started on the u-blox M8
      if (valfix == (maxvalfix - 1)) { // Only do this once
        Serial1.write(setLP, len_setLP);
      }
#endif
#endif

      if (valfix == maxvalfix) { // wait until we have enough valid fixes to avoid possible filename corruption

        // do the divides once to save time
        char GPShourT = GPS.hour/10 + '0';
        char GPShourU = GPS.hour%10 + '0';
        char GPSminT = GPS.minute/10 + '0';
        char GPSminU = GPS.minute%10 + '0';
        char GPSsecT = GPS.seconds/10 + '0';
        char GPSsecU = GPS.seconds%10 + '0';
        char GPSyearT = GPS.year/10 +'0';
        char GPSyearU = GPS.year%10 +'0';
        char GPSmonT = GPS.month/10 +'0';
        char GPSmonU = GPS.month%10 +'0';
        char GPSdayT = GPS.day/10 +'0';
        char GPSdayU = GPS.day%10 +'0';

        // build the gpx data string
        String gpx_dataString = "<trkpt lat=\"";
        gpx_dataString += String(GPS.latitudeDegrees, 6);
        gpx_dataString += "\" lon=\"";
        gpx_dataString += String(GPS.longitudeDegrees, 6);
        gpx_dataString += "\">\n";
        gpx_dataString += "<name>";
        gpx_dataString += String(trkpt);
        gpx_dataString += "</name>\n";
        gpx_dataString += "<time>20";
        gpx_dataString += GPSyearT; gpx_dataString += GPSyearU; gpx_dataString += "-";
        gpx_dataString += GPSmonT; gpx_dataString += GPSmonU; gpx_dataString += "-";
        gpx_dataString += GPSdayT; gpx_dataString += GPSdayU; gpx_dataString += "T";
        gpx_dataString += GPShourT; gpx_dataString += GPShourU; gpx_dataString += ":";
        gpx_dataString += GPSminT; gpx_dataString += GPSminU; gpx_dataString += ":";
        gpx_dataString += GPSsecT; gpx_dataString += GPSsecU; gpx_dataString += ".";
        gpx_dataString += String((int)GPS.milliseconds); gpx_dataString += "Z</time>\n";
        gpx_dataString += "<ele>";
        gpx_dataString += String(GPS.altitude, 1);
        gpx_dataString += "</ele>\n";
        gpx_dataString += "<fix>";
        if (!GPS.fix) {
          gpx_dataString += "none";
        }
        else {
          gpx_dataString += "3d";
        }
        gpx_dataString += "</fix>\n";
        gpx_dataString += "<sat>";
        gpx_dataString += String((int)GPS.satellites);
        gpx_dataString += "</sat>\n";
        gpx_dataString += "<hdop>";
        gpx_dataString += String(GPS.HDOP, 2);
        gpx_dataString += "</hdop>\n";
        gpx_dataString += "<desc>Battery voltage ";
        gpx_dataString += String(vbat, 2);
        gpx_dataString += "V</desc>\n";
        gpx_dataString += "</trkpt>\n";

        // build the csv data string
        String csv_dataString = "";
        csv_dataString += GPShourT; csv_dataString += GPShourU; csv_dataString += ':';
        csv_dataString += GPSminT; csv_dataString += GPSminU; csv_dataString += ':';
        csv_dataString += GPSsecT; csv_dataString += GPSsecU;
        //csv_dataString += '.'; csv_dataString += String(GPS.milliseconds, DEC); // comment this line out to ignore milliseconds
        csv_dataString += ' '; // separate time and date with a space (not a ',') to make life easier for numpy.loadtxt
        csv_dataString += GPSdayT; csv_dataString += GPSdayU; csv_dataString += '/';
        csv_dataString += GPSmonT; csv_dataString += GPSmonU; csv_dataString += "/20";
        csv_dataString += GPSyearT; csv_dataString += GPSyearU; csv_dataString += ',';
        csv_dataString += String(GPS.latitudeDegrees, 6); csv_dataString += ',';
        //csv_dataString += String(GPS.latitude, 4); csv_dataString += ','; csv_dataString += String(GPS.lat); csv_dataString += ',';
        csv_dataString += String(GPS.longitudeDegrees, 6); csv_dataString += ',';
        //csv_dataString += String(GPS.longitude, 4); csv_dataString += ','; csv_dataString += String(GPS.lon); csv_dataString += ',';
        csv_dataString += String(GPS.speed); csv_dataString += ',';
        csv_dataString += String(GPS.angle); csv_dataString += ',';
        csv_dataString += String(GPS.altitude); csv_dataString += ',';
        csv_dataString += String((int)GPS.fix); csv_dataString += ',';
        csv_dataString += String((int)GPS.fixquality); csv_dataString += ',';
        csv_dataString += String((int)GPS.satellites); csv_dataString += ',';
        csv_dataString += String(GPS.HDOP); csv_dataString += ',';
        csv_dataString += String(trkpt); csv_dataString += ',';
        csv_dataString += String(vbat, 2); csv_dataString += '\n';

        // flash red LED to indicate SD write (leave on if an error occurs)
        digitalWrite(RedLED, HIGH);

        // syntax checking:
        // check if voltage is > 3.55V, if not then don't try to write!
        if (vbat < 3.55) {
          fileReady = false; // Start new file when battery is recharged
          Serial.println("Low Battery!");
          return;
        }
        // check year
        if ((GPS.year < 16) || (GPS.year > 26)) return;
        // check month
        if (GPS.month > 12) return;
         // check day
        if (GPS.day > 31) return;
  
        if (!fileReady) {
          // filename is limited to 8.3 characters so use format: YYYYMMDD/HHMMSS.gpx
          gpx_filename[2] = GPSyearT;
          gpx_filename[3] = GPSyearU;
          gpx_filename[4] = GPSmonT;
          gpx_filename[5] = GPSmonU;
          gpx_filename[6] = GPSdayT;
          gpx_filename[7] = GPSdayU;
          gpx_filename[9] = GPShourT;
          gpx_filename[10] = GPShourU;
          gpx_filename[11] = GPSminT;
          gpx_filename[12] = GPSminU;
          gpx_filename[13] = GPSsecT;
          gpx_filename[14] = GPSsecU;
          
          csv_filename[2] = GPSyearT;
          csv_filename[3] = GPSyearU;
          csv_filename[4] = GPSmonT;
          csv_filename[5] = GPSmonU;
          csv_filename[6] = GPSdayT;
          csv_filename[7] = GPSdayU;
          csv_filename[9] = GPShourT;
          csv_filename[10] = GPShourU;
          csv_filename[11] = GPSminT;
          csv_filename[12] = GPSminU;
          csv_filename[13] = GPSsecT;
          csv_filename[14] = GPSsecU;

          dirname[2] = GPSyearT;
          dirname[3] = GPSyearU;
          dirname[4] = GPSmonT;
          dirname[5] = GPSmonU;
          dirname[6] = GPSdayT;
          dirname[7] = GPSdayU;
  
          // try to create subdirectory (even if it exists already)
          SD.mkdir(dirname);
          
          // Open the gpx file for writing
          gpx_dataFile = SD.open(gpx_filename, FILE_WRITE);
          
          // if the file is available, write to it:
          if (gpx_dataFile) {
            gpx_dataFile.print("<gpx>\n<trk>\n<trkseg>\n"); // write header
            gpx_dataFile.print("</trkseg>\n</trk>\n</gpx>\n"); // write footer
            gpx_dataFile.close();
            Serial.print("Logging to ");
            Serial.println(gpx_filename);
            delay(200); // Allow 0.2s for SD file to close
          }
          // if the file isn't open, pop up an error:
          else {
            Serial.println("Error opening GPX file (1)!");
          }
          
          // Open the csv file for writing
          csv_dataFile = SD.open(csv_filename, FILE_WRITE);
          
          // if the file is available, write to it:
          if (csv_dataFile) {
            csv_dataFile.print("HH:MM:SS DD/MM/YYYY,LAT,LON,SPEED,ANGLE,ALT,FIX,FIXQUAL,SAT,HDOP,TRKPT,BATV\n"); // write header
            csv_dataFile.close();
            Serial.print("Logging to ");
            Serial.println(csv_filename);
            fileReady = true;
            trkpt = 0;
            delay(200); // Allow 0.2s for SD file to close
          }
          // if the file isn't open, pop up an error:
          else {
            Serial.println("Error opening CSV file (1)!");
          }
        }

        if(fileReady) {
          // Open the gpx file for writing
          gpx_dataFile = SD.open(gpx_filename, FILE_WRITE);
          
          // if the file is available, write to it:
          if (gpx_dataFile) {
            unsigned long pos = gpx_dataFile.position(); // get file position
            gpx_dataFile.seek(pos - 24); // rewind to start of footer
            gpx_dataFile.print(gpx_dataString); // write new data
            gpx_dataFile.print("</trkseg>\n</trk>\n</gpx>\n"); // rewrite footer
            gpx_dataFile.close();
#ifdef DEBUG
            Serial.print(gpx_dataString);
#endif
            delay(200); // Allow 0.2s for SD file to close
          }
          // if the file isn't open, pop up an error:
          else {
            fileReady = false;
            Serial.println("Error opening GPX file (2)!");
          }
          
          // Open the csv file for writing
          csv_dataFile = SD.open(csv_filename, FILE_WRITE);
          
          // if the file is available, write to it:
          if (csv_dataFile) {
            csv_dataFile.print(csv_dataString);
            csv_dataFile.close();
#ifdef DEBUG
            Serial.print(csv_dataString);
#endif
            trkpt += 1; // increment track point
            digitalWrite(RedLED, LOW); // write was successful so turn LED off
          }
          // if the file isn't open, pop up an error:
          else {
            fileReady = false;
            Serial.println("Error opening CSV file (2)!");
          }
        }
      }
    }
  }
}

