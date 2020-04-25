# ESP32 ServoClock

[![ESP32 Servo Clock](https://github.com/3KUdelta/ESP32_ServoClock_MST/blob/master/servo_clock.jpg)](https://github.com/3KUdelta/ESP32_ServoClock_MST)\
Arduino code for digital clock, 3D printed inspired by www.otvinta.com, https://www.thingiverse.com/thing:3266949.
I added some thin back covers to hide the cables and added two feet to give a safe stand. These files are here: https://www.thingiverse.com/thing:4313748  

Author of the code: Jordi Ginjaume. Excellent code Jordi! It inspired me to do the thing with Arduino. Original Version 20190118 (his code and case can be found here: https://www.thingiverse.com/thing:3375870) 

Modified for use with an ESP32 wihtout RTC, just calling NTP all 12 hours over WiFi using the following hardware:

1 ESP32 programmed with Arduino\
2 PCA9685 chaining servo drivers, set solder blob on A0 on the second board for digits 3 and 4\
NTP call via WiFi\
28 SG90 servos

Important: keep sequence with servos plugging into PCA9685:\
first digit: 0-6, leave 7 blank\
second digit: 8-14, leave 15 blank\
repeat on second board for third and forth digit

PIN Connections on ESP32\
21 -- SLA\
22 -- SLC\
 0 -- OA
 
I inserted a switch to turn off the power to the PC9685 for programming purposes.

[![ESP32 Servo Clock](https://github.com/3KUdelta/ESP32_ServoClock_MST/blob/master/Bildschirmfoto%202020-04-05%20um%2010.16.57.png)](https://github.com/3KUdelta/ESP32_ServoClock_MST)

[![ESP32 Servo Clock](https://github.com/3KUdelta/ESP32_ServoClock_MST/blob/master/Bildschirmfoto%202020-04-05%20um%2010.18.04.png)](https://github.com/3KUdelta/ESP32_ServoClock_MST)
