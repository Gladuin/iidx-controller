# iidx-controller
Software that enables you to create your own Arduino based beatmania IIDX controller.

# Features
 - HID lighting support (that works with btools and ttools).
 - GUI configuration tool.
 - Multiple input methods (joystick, keyboard, mouse).

# Requirements
 - An Arduino Leonardo (technically compatible with any ATmega32U4 based board, but only tested with a Leonardo).
 - The [Bounce2](https://www.arduino.cc/reference/en/libraries/bounce2/) library.

# Setup
Buttons:
 - One terminal to GND and the other terminal to the corresponding pin.

LEDs:
 - Positive terminal to corresponding pin.
 - Negative terminal to GND.

Encoder:
 - Connect the encoder's wires to the corresponding pins.
 - Set `ENCODER_PPR` in `iidx-controller/config.h` to the PPR of your encoder.

Manufacturer name:
 - By setting `MF_NAME` in `iidx-controller/config.h`, you can change the manufacturer name of your controller.
 - You can change this to whatever you'd like, such as your name.

Info:
 - Pinouts are available in `iidx-controller/config.h`, you can edit them there if necessary.
 - Leonardo pinout (what the numbers in the codes pinout arrays mean) at the bottom of this page.
 - Additional settings can be configured via the GUI, more info [here](https://github.com/Gladuin/iidx-controller/tree/master/config-tool)

# Thanks
- Huge thanks to [CrazyRedMachine](https://github.com/CrazyRedMachine) for helping me out when I got stuck, and for their [SoundVoltexIO](https://github.com/CrazyRedMachine/SoundVoltexIO) repository.
- John Lluch's [Timer interrupt based Encoder library](https://github.com/John-Lluch/Encoder)
- The [digitalWriteFast library](https://github.com/watterott/Arduino-Libs/tree/master/digitalWriteFast)

# Leonardo pinout

![Leo pinout](https://raw.githubusercontent.com/Bouni/Arduino-Pinout/master/Arduino%20Leonardo%20Pinout.png)
