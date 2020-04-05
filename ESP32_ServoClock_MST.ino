/*
    Digital Clock
    Jordi Ginjaume
    Version 20190118
    Code started 20190110

    Arduino code for digital clock, 3D printed inspired by www.otvinta.com, using the following hardware:
    1 Arduino ESP32
    2 PCA9685 chaining servo drivers, set solder blob on A0 on the second board for digits 3 and 4
    NTP call via WiFi
    28 SG90 servos

    Modified for use with an ESP32 wihtout RTC, just calling NTP all 12 hours over WiFi
    Marc Stähli, modified April 2020

    Important: keep sequence with servos plugging into PCA9685:
    first digit: 0-6, leave 7 blank
    second digit: 8-14, leave 15 blank
    repeat on second board for third and forth digit
    Sequece per digit:   0
                       1   2
                         3
                       4   5
                         6

    PIN Connections on ESP32
    21 -- SLA
    22 -- SLC
     0 -- OA
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>    // Adafruit PCA9685 library
#include <WiFi.h>                       // For WiFi connection with ESP32
#include <WiFiUdp.h>                    // For NTP Signal fetch
#include <EasyNTPClient.h>              // For NTP Signal read https://github.com/aharshac/EasyNTPClient
#include <TimeLib.h>                    // For converting NTP time https://github.com/PaulStoffregen/Time.git

char ssid[] = "eiger1";                 // WiFi Router ssid
char pass[] = "Beloc-Zok-0123456789";   // WiFi Router password

// NTP
#define NTP_SERVER   "ch.pool.ntp.org"
#define TZ           1                  // (utc+) TZ in hours
#define TZ_SEC       ((TZ)*3600)        // remark: European DST will be calculated in the code
#define UTC_OFFSET + TZ

Adafruit_PWMServoDriver servoHours = Adafruit_PWMServoDriver(0x40);   //first PCA9685 address
Adafruit_PWMServoDriver servoMinutes = Adafruit_PWMServoDriver(0x41); //second PCA9685 address, solder blob on A0

WiFiUDP udp;
EasyNTPClient ntpClient(udp, NTP_SERVER, TZ_SEC);  //calls NTP for current timezone; summertime_EU() does the rest

//*** declaration of functions ***************************************************

void capturetime();
void showtime(byte m [4], int w);
void showdigit(byte i, byte digit, int w);
void go_online();
void get_NTP_time();
boolean summertime_EU(int year, byte month, byte day, byte hour, byte tzHours);

//*********************************************************************************

const int enablepin = 0;                  // ESP32 pin connected to the OE pin at PCA9685, would also work without
const int servopulse [2] {210, 450};      // pulse of servos at low positions and high position

const byte mapchar [12][7] = { //for each number, position of every segment
  {1, 1, 1, 0, 1, 1, 1}, // zero          //   0
  {0, 0, 1, 0, 0, 1, 0}, // one           // 1   2
  {1, 0, 1, 1, 1, 0, 1}, // two           //   3
  {1, 0, 1, 1, 0, 1, 1}, // three         // 4   5
  {0, 1, 1, 1, 0, 1, 0}, // four          //   6
  {1, 1, 0, 1, 0, 1, 1}, // five
  {1, 1, 0, 1, 1, 1, 1}, // six
  {1, 0, 1, 0, 0, 1, 0}, // seven
  {1, 1, 1, 1, 1, 1, 1}, // eight
  {1, 1, 1, 1, 0, 1, 1}, // nine
  {0, 0, 0, 0, 0, 0, 0}, // null (all down)
  {1, 1, 0, 1, 1, 0, 1}  // E for error indication
};

const byte servoreverse [4] [7] = {      // one identifies wservos that work reverse, zero is normal direction
  {1, 1, 0, 0, 0, 1, 1},
  {1, 1, 0, 0, 0, 1, 1},
  {1, 1, 0, 0, 0, 1, 1},
  {1, 1, 0, 0, 0, 1, 1}
};
const int servofinetune [4] [7] = {      // pulse to add to servopulse for fine tunning of each individual servo
  { -25, 0, 0, 10, -30, 0, -20},
  {0, -20, 0, -10, -55, -5, 30},
  {0, -20, -15, 0, 0, 0, 0},
  {20, 20, -10, 0, 10, -10, 10}
};
byte allon [4] = {8, 8, 8, 8};           // all digits on
byte alloff [4] = {10, 10, 10, 10};      // all digits off for startup
byte momentdisplay [4];                  // time shown in display
byte moment [4];                         // to store actual time in 4 digits
unsigned long timestamp;                 // timestamp to measure time past since last update on NTP server

void setup() {
  delay(500);                            // give time after boot
  Serial.begin(115200);
  pinMode(enablepin, OUTPUT);            // this is the pin for OE in the PCA (enable/disable servos LOW/HIGH)
  servoHours.begin();
  servoHours.setPWMFreq(60);             // set frequncy for first PCA
  servoMinutes.begin();
  servoMinutes.setPWMFreq(60);           // set frequency for second PCA
  digitalWrite(enablepin, LOW);          // enable the servos
  delay(200);
  showtime(allon, 220);                  // initializing all servos
  showtime(alloff, 220);
  go_online();                           // connect to WiFi
  get_NTP_time();                        // get precise time from time server
}

void loop() {
  capturetime();                         // assign time to digits
  showtime(moment, 220);                 // to the main routine that shows the time in our digital clock
  if ((now() - timestamp) > 43200) {     // 43200 = 12 hours
    get_NTP_time();                      // get all 12 hours a time update from NTP Server --> avoiding a constant read from time server
  }
}

void capturetime() {
  moment[0] = hour(now()) / 10;               // find out first digit from hour
  moment[1] = hour(now()) - moment[0] * 10;   // find out second digit from hour
  moment[2] = minute(now()) / 10;             // find our first digit from minute
  moment[3] = minute(now()) - moment[2] * 10; // find out second digit from minute
}

void showtime(byte m [4], int w) {
  int d = 275; // delay needed to stop the servos moving before disabling them
  // parameters: matrix with the digits to show
  if (memcmp(m, momentdisplay, 4) != 0) { // if time displayed is different to time expected to be shown
    for (int i = 0; i < 4; i++) { // for every single position, show only if different to actually shown
      if (m[i] != momentdisplay[i]) {
        showdigit(i, m[i], w);
        momentdisplay [i] = m[i]; // take note of the shown digit
      }
    }
    delay(d); // give time to the servos to stop moving
  }
}

void showdigit(byte i, byte digit, int w) { // parameters: i = digit position (0,1,2,3), digit = number, w = delay avoiding high power draw
  int pulse;
  byte servonum, a, segmentposition;

  for (byte j = 0; j < 7; j++) { // show the 7 segments
    servonum = j + (8 * ((i == 1 || i == 3))); // add 8 if position is 1 or 3 because 1 ands 3 digits start on pin 8 of the PCA
    segmentposition = mapchar[digit] [j]; // segment should be low or high?
    if (servoreverse [i] [j] == 0) {
      pulse = servopulse[segmentposition]; // for a normal (not reverse) servo assign pulse low or high
    }
    else {
      pulse = servopulse[!segmentposition]; // if servo works reverse, assign contrary
    }
    pulse = pulse + servofinetune [i] [j]; // add the fine tunning to the servo

    switch (i) { // switch for both PCA controllers
      case 0: case 1:
        servoHours.setPWM(servonum, 0, pulse);
        break;
      case 2: case 3:
        servoMinutes.setPWM(servonum, 0, pulse);
        break;
    }
    delay(w); // delay in between movement of contiguous segments, so we can do waves oleeeeeeee.
  }
}

void go_online() {
  WiFi.begin(ssid, pass);
  Serial.print("---> Connecting to WiFi ");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    i++;
    if (i > 20) {
      Serial.println("Could not connect to WiFi!");
      Serial.println("Doing a reset now and retry a connection from scratch.");
      showdigit(0, 11, 100);                  // E
      showdigit(2, 0, 100);                   // 0
      showdigit(3, 1, 100);                   // 1 : shows Error 01, could not connect to WiFi
      delay(5000);                            // wait 5 secs and then do a retry
      showtime(alloff, 220);
      go_online();                            // try again
    }
    Serial.print(".");
  }
  Serial.println("Wifi connected ok.");
} //end go_online

void get_NTP_time() {
  Serial.println("---> Now reading time from NTP Server");
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected! Doing that now again.");
    go_online();
  }
  int i = 0;
  while (!ntpClient.getUnixTime()) {
    delay(500);
    i++;
    if (i > 20) {
      Serial.println("Could not connect to NTP server!");
      Serial.println("Doing a reset now and retry a connection from scratch.");
      showdigit(0, 11, 100);                  // E
      showdigit(2, 0, 100);                   // 0
      showdigit(3, 2, 100);                   // 2 : shows Error 02, could not connect to NTP server
      delay(5000);                            // wait 5 secs and then do a reset
      resetFunc();                            // doing a full reset to retry
    }
    Serial.print(".");
  }

  setTime(ntpClient.getUnixTime());           // get UNIX timestamp (seconds from 1.1.1970 on)

  if (summertime_EU(year(now()), month(now()), day(now()), hour(now()), 1)) {
    adjustTime(3600);                         // adding one hour
  }
  timestamp = now();
} // end get_NTP_time()

boolean summertime_EU(int year, byte month, byte day, byte hour, byte tzHours)
// European Daylight Savings Time calculation by "jurs" for German Arduino Forum
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
// return value: returns true during Daylight Saving Time, false otherwise
{
  if (month < 3 || month > 10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
  if (month > 3 && month < 10) return true;  // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
  if (month == 3 && (hour + 24 * day) >= (1 + tzHours + 24 * (31 - (5 * year / 4 + 4) % 7)) || month == 10 && (hour + 24 * day) < (1 + tzHours + 24 * (31 - (5 * year / 4 + 1) % 7)))
    return true;
  else
    return false;
} // end of summertime_EU()
