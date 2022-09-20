# bce-infrared-receiver
Infrared receiver library example

This example will send JSON messages with decoded infrared packets. Right now only NEC protocol is supported.

Connect Core module by USB to your computer and connect TSOP4838 output pin to the P10 pin on Core Module.
Make sure that you have correct power supply for your receiver - 3 or 5V.
External pull-up on sensor OUT pin is not neccessary.
Internal pull-up in MCU is enabled.
