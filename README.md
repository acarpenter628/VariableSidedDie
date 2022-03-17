# VariableSidedDie
Arduino based dice simulator

This project is an electrical replacement for the various dice used in TTRPGs (Dungeons and Dragons, etc).  

This was a project I started roughly five years ago and cleaned up recently so that my GitHub would have things on it.  Right now this is just an Arduino breadboard project - I originally lost interest when I had to start thinking about mechanics.  Maybe someday it could be a kickstarter project, but DnD has been cool for years now and I probably waited too long to cash in.

**Features:**

 * Tap the accelerometer to 'roll' the die
 * Press the button to change the number of sides on the die

**Build Instructions**

Copy the contents of the `libraries` folder into your Arduino libraries folder (probably `C:\Users\[username]\Documents\Arduino\libraries\`).  A lot has changed in Arduino since I wrote this, apparently.  If you try to use the library manager, you will run into [this](https://github.com/adafruit/Adafruit-GFX-Library/issues/88) issue at the very least.  I commented out the only function in Adafruit_Sensor.cpp instead of dealing with version conflicts.

**Future Improvements:**

Short term:
 * Look into how to interface with the SD card and replace the pixelated default font with images
 * Handle cycling through dice type faster - balance it with the fade out indication.  Right now one or the other works
 * Add fudge dice?  Does anybody use those?

Longer term:
 * Investigate power/battery needs
 * Look into mechanics (laser cut plexiglass, maybe)
 * Transition away from Arduino and Adafruit parts
   * ST MCUs and Bosch accelerometers are pretty cheap, but the display will require more investigation

