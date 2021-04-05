FIRST!!!!

Check this encredible Synths and Drum-Computer from Marcel License:
https://github.com/marcel-licence

If You already played around with some other libraries, - the sound is really as good as in the videos.
I have tested the synth and the drum-computer, but as I have no MIDI-Controller-Keyboard, I was not able to test the Drum-Computer as it is.


This Sampleplayer based on the Drum_Computer created by Marcel License has some differences:
- I dropped the Sequencer to lower the complexity
- it is not optimized for a specific MIDI-Keyboard - I have removed this part of code to lower the complexity
- 4 optional Pots on (GPIO 32-35) could be added ( I did not test this because I am waiting for the postman with the packet )
- 5 very simple drumkits are included and it provides some kind of program-change via MIDI. These samples were sampled by myself or extracted from other free samplepacks for the Novation-Circuit.
- I used a PCM5102 DAC on Ports 25 BCLK, 27 WCLK and 26 for DOUT/DIN
- I used a 6N139 with some resistors as the MIDI-IN. I have planned to connect this Drum-Synth directly to a Volca Beats internal MIDI-Out using 2 resistors to split 5Volts MIDI into 3.3 Volts MIDI.
- The integration with the Volca Beats is not yet tested. I will post results later.
- To be able to add this amount of samples, I had to lower the samplerate of many sounds. Some are sampled by 22.050 Hz and others by 32.000 Hz. All Sounds are MONO!

Not implemented in the code:
- CC-Controllers to change an individual Sound. There are some Functions to control the Decay and the Pitch for each of the 12 Samplesounds.
- theoretically, with some Analog-Multiplexers You could build an anaolog UI which looks like a drum-computer. 

Not tested:
- I2S-NoDac-Option

Ideas:
- Adding PAN, Delay, LP/HP-Filter per Sample
- Analog UI with a 12 sliders, and Pots per Sound
- Analog-Inputs to control the sounds via a modular setup
- 

