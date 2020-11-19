#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#include <Adafruit_GPS.h>

// what's the name of the hardware serial port?
#define GPSSerial Serial1

#define MODE_BUTTON 14

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

Adafruit_AlphaNum4 alpha0 = Adafruit_AlphaNum4();
Adafruit_AlphaNum4 alpha1 = Adafruit_AlphaNum4();

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO false

uint32_t timer = millis();

enum SystemMode {
  METERS, 
  FEET, 
  SATS,
  GRID
};

SystemMode current_mode;
int previous_button_value;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MODE_BUTTON, INPUT_PULLUP);
  current_mode = METERS;
  previous_button_value = HIGH;
  
  alpha0.begin(0x70);
  alpha1.begin(0x71);

  // while (!Serial);  // uncomment to have the sketch wait until Serial is
  // ready

  // connect at 115200 so we can read the GPS fast enough and echo without
  // dropping chars also spit it out
  Serial.begin(115200);
  Serial.println("Adafruit GPS library basic test!");

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data)
  // including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  // GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or
  // RMC+GGA since the parser doesn't care about other sentences at this time
  // Set the update rate (uncomment the one you want.)
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  // GPS.sendCommand(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ); // 5 second update
  // time
  // GPS.sendCommand(PMTK_SET_NMEA_UPDATE_100_MILLIHERTZ); // 10 second update time
  // For the parsing code to work nicely and have time to sort thru the data,
  // and print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  // GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);

  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);
}

char buffer[9];

void loop() // run over and over again
{
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c)
      Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    // Serial.println(GPS.lastNMEA()); // this also sets the newNMEAreceived()
    // flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag
                                    // to false
      return; // we can fail to parse a sentence in which case we should just
              // wait for another
  }

  // approximately every 2 seconds or so, random intervals, print out the
  // current stats
  static unsigned nextInterval = 2000;
  if (millis() - timer > nextInterval) {
    timer = millis(); // reset the timer
    nextInterval = 2000;
    // Time in seconds keeps increasing after we get the NMEA sentence.
    // This estimate will lag real time due to transmission and parsing delays,
    // but the lag should be small and should also be consistent.
    float s = GPS.seconds + GPS.milliseconds / 1000. + GPS.secondsSinceTime();
    int m = GPS.minute;
    int h = GPS.hour;
    int d = GPS.day;
    // ISO Standard Date Format, with leading zeros https://xkcd.com/1179/
    Serial.print("\nDate: ");
    Serial.print(GPS.year + 2000, DEC);
    Serial.print("-");
    if (GPS.month < 10)
      Serial.print("0");
    Serial.print(GPS.month, DEC);
    Serial.print("-");
    if (d < 10)
      Serial.print("0");
    Serial.print(d, DEC);
    Serial.print("   Time: ");
    if (h < 10)
      Serial.print("0");
    Serial.print(h, DEC);
    Serial.print(':');
    if (m < 10)
      Serial.print("0");
    Serial.print(m, DEC);
    Serial.print(':');
    if (s < 10)
      Serial.print("0");
    Serial.println(s, 3);
    Serial.print("Fix: ");
    Serial.print((int)GPS.fix);
    Serial.print(" quality: ");
    Serial.println((int)GPS.fixquality);
    Serial.print("Time [s] since last fix: ");
    Serial.println(GPS.secondsSinceFix(), 3);
    Serial.print("    since last GPS time: ");
    Serial.println(GPS.secondsSinceTime(), 3);
    Serial.print("    since last GPS date: ");
    Serial.println(GPS.secondsSinceDate(), 3);
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4);
      Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(GPS.longitude, 4);
      Serial.println(GPS.lon);
      Serial.print("Speed (knots): ");
      Serial.println(GPS.speed);
      Serial.print("Angle: ");
      Serial.println(GPS.angle);
      Serial.print("Altitude: ");
      Serial.println(GPS.altitude);
      Serial.print("Satellites: ");
      Serial.println((int)GPS.satellites);
    }
    Serial.print("Input button: ");
    Serial.println(previous_button_value);
  }
  switch(current_mode){
    case METERS:
      show_altitude_meters(buffer);
      break;
    case FEET:
      show_altitude_feet(buffer);
      break;
    case SATS:
      show_number_of_satellites(buffer);
      break;
    case GRID:
      show_grid(buffer);
      break;
  }
  write_buffer(buffer);
  int button_value = digitalRead(MODE_BUTTON);
  if(button_value == HIGH && previous_button_value == LOW){
    current_mode = switch_mode(current_mode);
  }
  previous_button_value = button_value;
}

SystemMode switch_mode(SystemMode current){
  switch(current){
    case METERS:
      return FEET;
    case FEET:
      return SATS;
    case SATS:
      return GRID;
    case GRID:
      return METERS;
    default:
      return METERS;  
  }
}

#define valueUpperA 'A'
#define value0 '0'
#define valueA 'A'

void show_grid(char *buf){
  buf[0] = ' ';
  buf[1] = ' ';
  if(GPS.fix){
    int str_offset = 2;
    double latitude = GPS.latitudeDegrees;
    double longitude = GPS.longitudeDegrees;
    int long_offset = (int)((longitude + 180.0)/20.0);
    char c = (char)(valueUpperA + long_offset);
    buf[str_offset++] = c;
    int lat_offset = (int)((latitude + 90.0)/10.0);
    c = (char)(valueUpperA + lat_offset);
    buf[str_offset++] = c;
    int long_square = (int)((longitude + 180.0 - 20 * long_offset)/2.0);
    c = (char)(value0 + long_square);
    buf[str_offset++] = c;
    int lat_square = (int)(latitude + 90.0 - 10 * lat_offset);
    c = (char)(value0 + lat_square);
    buf[str_offset++] = c;
    int long_subsquare =  (int)(12.0*(longitude + 180.0 - 20.0 * long_offset - 2.0 * long_square)); // 5' is 1/12 of a degree
    c = (char)(valueA + long_subsquare);
    buf[str_offset++] = c;
    int lat_subsquare = (int)(24.0*(latitude + 90.0 - 10 * lat_offset - lat_square)); // 2.5' is 1/24 of a degree
    c = (char)(valueA + lat_subsquare);
    buf[str_offset++] = c;
  } else {
    buf[2] = 'N';
    buf[3] = 'O';
    buf[4] = ' ';
    buf[5] = 'F';
    buf[6] = 'I';
    buf[7] = 'X';
  }
}

void show_number_of_satellites(char *buf){
    int sats = (int)(GPS.satellites);
    sprintf(buf, "SATS  %2d", sats);
}

void show_altitude_meters(char *buf){
    int meters = (int)(GPS.altitude + 0.5);
    sprintf(buf, "%7dM", meters);
}

void show_altitude_feet(char *buf){
    int feet = (int)(GPS.altitude/.303 + 0.5);
    sprintf(buf, "%7dF", feet);
}

void write_buffer(char *buf){
  if(strlen(buf) < 8){
    return;
  }
  alpha0.writeDigitAscii(0, buf[0]);
  alpha0.writeDigitAscii(1, buf[1]);
  alpha0.writeDigitAscii(2, buf[2]);
  alpha0.writeDigitAscii(3, buf[3]);

  alpha1.writeDigitAscii(0, buf[4]);
  alpha1.writeDigitAscii(1, buf[5]);
  alpha1.writeDigitAscii(2, buf[6]);
  alpha1.writeDigitAscii(3, buf[7]);
  // write it out!
  alpha0.writeDisplay();
  alpha1.writeDisplay();
}
