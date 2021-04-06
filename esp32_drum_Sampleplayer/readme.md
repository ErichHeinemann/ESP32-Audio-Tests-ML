FIRST!!!!

Check this encredible Synths and Drum-Computer from Marcel Licence:
https://github.com/marcel-licence

If You already played around with some other libraries, - the sound is really as good as in the videos.
I have tested the synth and the drum-computer, but as I have no MIDI-Controller-Keyboard, I was not able to test the Drum-Computer as it is.

Therefore

This Sampleplayer based on the Drum_Computer created by Marcel License has some differences:
- I dropped the Sequencer to lower the complexity
- I replaced the custom MIDI-Handlers by the well documented Arduino MIDI Library (Forty Seven Effects)
- it is not optimized for a specific MIDI-Keyboard - I have removed this part of code to lower the complexity
- 4 optional Pots on (GPIO 32-35) could be added ( I did not test this because I am waiting for the postman with the packet )
- 5 very simple drumkits are included and it provides some kind of program-change via MIDI. These samples were sampled by myself or extracted from other free samplepacks for the Novation-Circuit.
- I used a PCM5102 DAC on Ports 25 BCLK, 27 WCLK and 26 for DOUT/DIN
- I used a 6N139 with some resistors as the MIDI-IN. I have planned to connect this Drum-Synth directly to a Volca Beats internal MIDI-Out using 2 resistors to split 5Volts MIDI into 3.3 Volts MIDI.
- The integration with the Volca Beats is not yet tested. I will post results later.
- To be able to add this amount of samples, I had to lower the samplerate of many sounds. Some are sampled by 22.050 Hz and others by 32.000 Hz. All Sounds are MONO!
- To play these samples with their lower samplerate in the right pitch, I  implemented a simple correction based on expected and real samplerate and change the pitch based on this calculation
- some optional functions like Analog-Inputs or Debugging could be enabled/disabled by the classic defines: like #define DEBUG_MIDI in midi_interface.ino


Not implemented in the code:
- CC-Controllers to change an individual Sound. There are some Functions to control the Decay and the Pitch for each of the 12 Samplesounds.
- theoretically, with some Analog-Multiplexers You could build an anaolog UI which looks like a drum-computer. 

Not tested:
- I2S-NoDac-Option

Known Problems:
The additional installation of the ESP32-Littlefs-Library took some hours. I have given up on my MAC and restarted an old Windows-Notebook to use it.
While using the Littlefs-Tools, close all Serial-Monitors!!!! otherwise it won´t work.

Installation:
- Download all files including the subdirectory "data" and the wav-files
- Install all needed libraries
- Open Arduino IDE ( min 1.8.13 ?)
- Connect a ESP32
- Select the right Module with 4MB minimum (ESO32 Dev Module) .. I used the defaults which worked:
- Settings for the board: Falsh Frequency: 80MHz, Flash-Size: 4MB, Partition Scheme Default 4MB with spiffs(1.2MB&1.5MB) or "No OTA (1MB APP/3 MP SPIFFS)
- PSRAM was disabled, - perhaps my old modules don´t have PSRAM?
- Select the right COM-Port ( connect/disconnect the ESP32 and You will identify the right port! )
- Press Upload 

One simple hint for testing MIDI with the Optocoupler:
I replaced the 6N139 with an LED on Pin 2 for the Anode and Pin3 for the Cathode. That way, the LED blinks if Your DIN 5 Pin-Connector is well connected.

Ideas:
- Adding Volume, Pan, Delay, LP/HP-Filter per Sample
- Analog UI with a 12 sliders, and Pots per Sound
- Analog-Inputs to control the sounds via a modular setup
- Touch-Sensors to create something like a Wavedrum or Art-Installations
- sampling??

