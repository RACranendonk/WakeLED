# WakeLED
Arduino LED wake-up light. Uses the Arduino Uno and an Ethernet Shield. Built with the Ikea Dioder LED-strip, but can be used with any common-anode RGB LED-strip. See [here](http://i.imgur.com/TOHI6ij.png) for the wiring diagram.

The WakeLED is controlled via an Android application which sends the requests to the device.

Please use this at your own risk! I'm not to be blamed for any missed meetings due to the alarm freezing at night ;-)

## HTTP branch
The HTTP branch features the Arduino hosting a tiny webserver to which the app sends requests, which are then handled by parsing the messages.

## UDP branch
Similar to the HTTP branch, but uses UDP messages instead of HTTP get-requests, which is more efficient.

## Issues
Currently there are some issues with the WakeLED freezing at random intervals, switching to UDP might fix this issue, but I haven't tested this yet.
