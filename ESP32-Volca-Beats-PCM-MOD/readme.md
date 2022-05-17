!!! It compiles only with the Arduino IDE 1.8.19, don´t try it with Arduino 2.0 !!!
I do not have the time to keep it compatible with everything!
If it doesn´t compile, You have to search the problem by Yourown.
Try it first on a breadboard and afterwards make it robust and solid.
If still all works, then modify Your Volca.

Yes, there are makers outside who first drilled the holes and then they figured that they bought the wrong ESP32-Modules...

As user try to let me take down my code, he want me to delete it. But as Github works, I keep it only.
I have tried this code with different I2S-PCM-Cards and ESP32 Devkits and it works fine.

The Drum-Synth Sketch was used to MOD a Volca Beats.
The Volca Beats (Korg TM) now has two different Sounds-Sources, 
- first the original Sounds and 
- second a set of 10 PCM-Sounds partially controlled by the same Controls as the normal UI comming from the ESP32

Not all possible functions are implemented yet but it works and creates some new colors of sound.
The Volca Beats is sending some more CCs via MIDI but I believe I have added the most usable ones to the code of the ESP32.

I first connected the Audio-Out of the PCM5102 to the Volume-Pot of the Korg Volca Beats. But to mix both soundsource independently, I decided to cut that wire and use 2 channels on my mixer for the Volca Beats, one for the old Sounds and one channel for the new sounds from the ESP32.

The best step would be to go further to use an extra box for the new Sound-Source and only connect the MIDI-Out (TX/GND) from the Volca to this new box.

A short Video:
https://www.youtube.com/watch?v=XIrn2-dZn1U


Also take alook to the Code from the main developer: 

Marcel Licence:
https://github.com/marcel-licence
