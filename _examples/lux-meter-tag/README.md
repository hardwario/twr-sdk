# Lux meter tag Example

This example shows simple application to use Lux meter tag and send measured data over USB.


## Requirements
  - HARDWARIO Core module
  - HARDWARIO Lux meter tag


## Principle
  - Lux meter is set to work asynchronous
  - update interval is 1000 milliseconds
  - when update is triggered, measured data are send over USB in format
  ```dddddd.ddddd``` (ended with ```\r\n```)
  - if error occures during update process, string defined in ```LUXMETER_ERROR_MSG``` is send instead of a number
