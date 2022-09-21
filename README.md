[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

# esp8266-ecowatt
An IoT ecowatt notification display that displays the French electricity network health previsions for the next 5 hours on 5 groups of 3 LEDs (red, orange, green).

Uses the [RTE (Réseau de Transport d'Électricité) API](https://data.rte-france.com/catalog/-/api/consumption/Ecowatt/v4.0)
You will need to create your own account and subscribe to this API in order to use this project.
Note that the code used for the OAuth2 authentication here will work on the other data.rte-france.com APIs.

The 15 LEDs are controlled with 10 digital outputs only through a little hack since there are not enough output pins available on the ESP8266. This would have been easier with a demultiplexer, and this limits the control I have over the LEDS, I can't light all of them at the same time for example. But as long as I can turn one LED On and the others Off in each group of 3 LEDs, I'm good.

This is an Arduino Project to be built in the Arduino IDE for the ESP8266 microcontroller.
It could easily be ported over ESP32 if needed, or modified to use a screen instead of the LEDs. I chose the LEDs because I thought it was a more user-friendly interface than a monochrome screen.

![alt text](https://github.com/Abela-Things/esp8266-ecowatt/blob/master/IMG_20220921_120627.jpg)
