# Arduino Gamepad/Mouse/Keyboard controller

#### Version 1.0

This project was created with the idea of obtaining an additional gamepad to use in some games and at the same time having the mouse in joystick format.
Additionally, a switch was added that goes from gamepad mode to keyboard mode. This means that when you press the buttons the computer receives that the F13 to F24 button was pressed. Which is great for controlling programs like OBS Studio where they can be used as hotkeys for various things.
When you go from one mode to the other, the mouse continues to work.

### Libraries Used
* Joystick
* Keyboard
* Keypad
* Mouse

## Materials needed
- Arduino Leonardo (can use another type of arduino but HID capability is needed)
- 9 Push button switches
- 3 Rotary encoders
- 4 RCA Jacks
- 1 Joystick with switch (I use the KY-023 Joystick)
- 1 SPST Rocker Switch

## Schematics

![Schematic Image](https://raw.githubusercontent.com/gkoutian/arduino-gmk-controller/main/img/schematic.png)

## Connection drawing

![Connection drawing image](https://raw.githubusercontent.com/gkoutian/arduino-gmk-controller/main/img/connection-drawing.png)

## Example of the type of box you can make

![Box example image](https://raw.githubusercontent.com/gkoutian/arduino-gmk-controller/main/img/render.png)
