# Climate Module Example

This example shows how to use Climate module and send measured data over USB.
**Be aware** that using USB stack (to send data to PC) causes the microcontroller not going to sleep mode - temperature data can be slightly affected.


## Requirements
  - HARDWARIO Core module
  - HARDWARIO Climate module


## Principle
  - Climate module is set to work asynchonous (using the HARDWARIO SDK)
  - update interval is 2500 milliseconds
  - when update is triggered, measured data are send over USB as float values
  separated with comma and space (for better readability)
