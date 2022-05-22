# config-tool
Configure your arduino-based controller from the comfort of a gui!

# Installing
Get the latest zip from [insert releases url here], extract, and run the exe. Simple as that!

# What does what?
| Option                 | Explanation                                                                                                                                                                                                                                                                                                                                     |
|------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Controller mode        | Defines what your controller behaves as.<br>If set to Joystick it will behave as, well, a joystick.<br>If set to Keyboard it will send keystrokes to your computer.                                                                                                                                                                             |
| LED mode               | Defines how your leds behave.<br>Reactive + HID will switch from HID mode to Reactive if no HID LED commands are sent in 3 seconds.<br>HID will make the leds only react to HID commands.<br>Reactive will make the leds react to button presses.<br>Off will ignore everything and the leds will not light up (except for the boot animation). |
| TT mode                | Defines how your turntable behaves.<br>Analog will report your turntable state as an incrementing or decrementing variable<br>Digital will report your turntable as either moving up or down.<br><br>If in keyboard mode analog will make the turntable show up as a mouse.                                                                     |
| TT increments per turn | Defines how many increments are needed to reach a full 360° turn. Limited from 1 to 255.<br><br>Set to 72 for 60Hz IIDX, 144 for 120Hz IIDX, 255 for 1 : 1.                                                                                                                                                                                     |
| Debounce time          | Defines how many milliseconds are used to debounce your buttons. Limited from 1 to 255.                                                                                                                                                                                                                                                         |
| Polling rate           | Defines how fast the controller sends its reports to the computer.                                                                                                                                                                                                                                                                              |

# Building
I used python 3.10 to create this, so it definitely works with that version. YMMV with other versions.

0. (Optional) Create a python VENV and activate it. (https://docs.python.org/3/library/venv.html)[See here how]
1. Install the required packages by running `pip install -r requirements.txt`
2. Build the executable by running `pyinstaller --onefile --noconsole controller-config.py`
3. Done.