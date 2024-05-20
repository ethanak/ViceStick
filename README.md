# ViceStick
ESP32 bluetooth joystick for VICE Commodore emulator

![Unit view](/images/view.jpg)

## Short description
Originally this unit was made with ESP32 Lolin32 Lite version, and schematic diagram is for this board. See file ViceStick.h in folder ViceStick for pin description.

![Connection diagram for Lolin32 Lite board](/images/schematic.png)

Code was written for Arduino IDE.

## Controls

There are three extra buttons:

* Autofire
  * Short click - turn on/off autofire
  * Press for more than 0.5 sec - change autorire period
* Program
  * Short click - change current settings
  * Long press - change global options
* Port
  * Short click - show port (1 or 2)
  * Long click - change port

## Current settings

* Fire buttons - straight or swapped
* Joystick rotation - plain, left, back, right
* Run/Stop and Space on Fire2 on/off - if not in keyset, sends Esc (short click) or Space bar code (longer press)
* Autofire period - 100 to 1000 msec in 10 steps
* Autofire mode - press (only if fire button is pressed) or still
* Autofire On/Off
* Port - 1 or 2
* Direction - 8 (standard) or 4 (only single Up, Down, Left, Right direction
 keys will be sent)
* Keyset1 and Keyset2 - select predefined numpad or one of programmed keysets for port 1 and 2
* Bluetooth name - select one of programmed names
* Copy from - copy setting from selected preset

Press Program button to store or right fire button for cancel. Automatic cancel will be called 10 seconds after last activity

## Global settings

* Keyset (1 or 2) - set key for each direction. If Fire2 is not set, acts as Fire. If semi-dirs are not set, keeys combination will be send to Vice.
* Preset (1 to 5):
  * Fire buttons - straight or swapped
  * Autofire period - 100 to 1000 msec in 10 steps
  * Autofire mode - press (only if fire button is pressed) or still
  * Autofire On/Off
  * Port - 1 or 2
  * Direction - 8 (standard) or 4 (only single Up, Down, Left, Right direction keys will be sent)
  * Keyset1 and Keyset2 - select predefined numpad or one of programmed keysets for port 1 and 2
  * Bluetooth name - select one of programmed names
  * Copy to - copy curent settings to preset
  * Name - change name (up to 4 characters(
* Name (1 or 2) - set bluetooth name (up to 15 characters)

Use joystick Up/Down to select setting, Right/Left to select number. In text/keyset edition use Right/Left to select position, Up/Down to change character, Fire to accept. Use right fire button to cancel.

## USB mode

Only for Linux. Connect joystick to your PC with standard USB cable and run program linux/joykey.py with port name (like ttyUSB0) or USB id (like 1a86:7523).

