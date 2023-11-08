# ESP32 Bluetooth Page Turner

## What it is for.
This is the code for a Bluetooth enabled page turner for use by musicians and
others that need a foot operated method of turning pages on a device.

## Requirements
### Hardware
You will require an ESP32 board with USB and battery support.
I used a Lolin D32. Other boards may be suitable.

The external hardware is pretty simple, just connect momentary normally open (NO)
buttons or footswitches between each of the NEXT_PAGE and PREVIOUS_PAGE GPIO
pins and ground.

The D32 board already has a LED connected to GPIO 5, if yours does not
have one then connect a LED with a suitable resistor (~2k) between that GPIO
and the 3.3V rail.

Add a lithium ion battery and put it all in a case then you are sweet.

### Software
This software is Arduino code designed for the ESP32.
You will need to install ESP32 support in the Arduino IDE.
<https://docs.espressif.com/projects/arduino-esp32/en/latest/getting_started.html>

You will need to install two libraries:
 * [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino/tree/release/1.4)
 * [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard)

## Options
The #defines in the first part of the code give you some options that you can
tweak to your liking.

## Operation
The device will start up and immediately go into deep sleep. This is to conserve
battery life. Press the wakeup button (by default the Previous Page button) in
order to wake it. You may now pair to a device.

The device acts as a Bluetooth keyboard, on my tablet I had to re-enable the
onscreen keyboard when I paired with this.

Most score reading software accepts the right and left arrow keys to turn pages.
You can change that in the code if you need.

If the device remains unconnected for more than 2 minutes (configurable) then
it will go back to sleep again and requires a button press to restart. The
LED indicates when the device has a bluetooth connection.
