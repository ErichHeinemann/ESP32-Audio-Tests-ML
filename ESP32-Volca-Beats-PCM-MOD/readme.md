The Drum-Synth Sketch was used to MOD a Volca Beats.
The Volca Beats (Korg TM) now has two different Sounds-Sources, 
1. the original Sounds and 
2. 2. a set of 10 PCM-Sounds partially controlled by the same Controls as the normal UI comming from the ESP32

Not all possible functions are implemented yet but it works and creates some new colors of sound.
The Volca Beats is sending some more CCs via MIDI but I believe I have added the most usable ones to the code of the ESP32.

I first connected the Audio-Out of the PCM5102 to the Volume-Pot of the Korg Volca Beats. But to mix both soundsource independently, I decided to cut that wire and use 2 channels on my mixer for the Volca Beats, one for the old Sounds and one channel for the new sounds from the ESP32.

The best step would be to go further to use an extra box for the new Sound-Source and only connect the MIDI-Out (TX/GND) from the Volca to this new box.

A short Video:
https://www.youtube.com/watch?v=XIrn2-dZn1U

Also check the Code from the main developer: 

Marcel Licence:
https://github.com/marcel-licence
