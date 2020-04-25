# ESP32 ServoClock
Author: Jordi Ginjaume, Original Version 20190118 (https://www.thingiverse.com/thing:3375870), Modified by Marc Staehli, 2020

Arduino code for digital clock, 3D printed inspired by www.otvinta.com, https://www.thingiverse.com/thing:3266949 using the following hardware:

  1 ESP32 programmed with Arduino
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

[![ESP32 Servo Clock](https://github.com/3KUdelta/ESP32_ServoClock_MST/blob/master/servo_clock.jpg)](https://github.com/3KUdelta/ESP32_ServoClock_MST)

