<h1> DIY Volca - <b>POlca</b></h1>

Video 1: - bad Video-Quality (geman)
https://www.youtube.com/watch?v=xNPJgFerRqw

Video 2: Bad Audio ( very low volume )
https://www.youtube.com/watch?v=J7IOvEoxopA

<b>What will Video 3 provide?</b>

I would like to create a UI for an electronic Instrument based on ESP32 only using I2C for a Display (SSD1306) 16 Step-Buttons, 16 Leds, some Control-Buttons and 4 Potentiometers and finally a Rotary Encoder. The Rotary-Encoder - a standard from the nineties ist more like  a nightmare on I2C. I use only Core0 for the I2C! That way, Core 1 - the default for Arduino is free for Your application! Core 0 supports I2C only with 50KHz - which is less then expected. To get all these GPIO or GPI - Ports, I am using 3x PCF8574 I2C multiplexers which could toggle between Input-muxer and Output-muxer to drive the LEDs. The Rotary-Encoder is connecteed to the last 3 Pins of the third PCF7574.  The code is optimized to request the status of the third PCF8574 as often as possible to get any turn of the encoder. In the current code it is still not optimal.
I will add some screenshots to get an overview how it looks alike in reality.
I named my project, a 16 Stepsequencer with an Sample-Engine "Polca" a combination of the "PO"-Series and the "Volca" - Series.
Around 20. July 2021 I uploaded 12 Percussion-Samples and it works well as a good add on to a Novation Circuit with MIDI-Sync (Midi Clock).

Why only using I2C?
I try to move to the AI Thinker Audio 2.2 -board and this has less free GPIOs. I was happy to get I2C on this board by removing 2 Resistors!
That way, I left a lot of GPIOs unused. If You use a normal ESP32 You could add a lot by Your own.

Final statement:
This code is not for a final product, its purpose is only for testing the capabilities with I2C on Core 0 on ESP32.

Userguide:
While playing with an Akai MPC 2k5 I have learned that the Pads could be used as a Menu and I mostly don´t use the Rotary Encoder of it.
I tried to make the UI mostly usable without Rotary Encoders. I use 2 Function-Buttons, F1 to select the Tracks or Instruments, F2 as the Global Function to get access to other menus. Potentially, I could create 16 Menus which could be accessed using F2 and one of the 16 Step-Buttons.
The userguide is in the folder "/docs/".

How are the WAV-Files stored?
I used LittleFS which is a AddOn for ESP32. create a folder "data" inside the project-folder
Create a new folder "0" inside /data for the first 12 WAV-Files for the first Set
and add the WAV-Files with following specs:
- 16Bit
- Mono
- 44.1 kHz Samplingrate
- Filename must be something like: 100_LCO.WAV where "LCO" will be displayed in the OLED-display, the Numbers should be in a order.
In the moment the ESP32 boots, it loads the samples into its sampleslots in the order of the samplenames.
In my case I have this structure of files:
/data/0/100_LCO.wav
/data/0/101_MCO.wav
/data/0/100_HCO.wav
/data/0/100_LTI.wav
...
/data/0/100_LWH.wav

LCO for Low Conga, MCO for Middle Conga etc.

I am planning to support 5 Soundsets in the subfolder /data/0 to /data/4.

Does it manage Midi or create any Sounds: 
Yes, it uses MIDI CLock to Sync and it creates sounds since around 20.July 2021

<h3>Minor Changes and Milestones:</h3>
2021-08-03 Accent works well, Display optimized to show the values in an acceptable way
2021-08-15 Change of Soundset works, Reverb works, Delay works and alle effects are controlable by the Pots and Buttons, SSD1306 and SH1106 are both supported. I replaced the small 0.96" SSD1306 display with a 1.3" SH1106!

<hr>
<h3>Lab-Board Components</h3>
<img src="https://github.com/ErichHeinemann/ESP32-Audio-Tests-ML/raw/main/ESP32Core0_TEST_I2C_ssd1306_ADS11115_MIDI_PCF8574/ESP32%20Step%20Sequencer.png">

Lab-Board on 3d-printed holder base, the PCF8574 DIL are under the OLED
Testing the UI with the OLED - 16 Steps as bubbles
<img src="https://github.com/ErichHeinemann/ESP32-Audio-Tests-ML/raw/main/ESP32Core0_TEST_I2C_ssd1306_ADS11115_MIDI_PCF8574/IMG_5640.JPG">
Testing 4 Valuees with 4 Pots (ADS1115)
<img src="https://github.com/ErichHeinemann/ESP32-Audio-Tests-ML/raw/main/ESP32Core0_TEST_I2C_ssd1306_ADS11115_MIDI_PCF8574/IMG_5634.JPG">

<img src="https://github.com/ErichHeinemann/ESP32-Audio-Tests-ML/raw/main/ESP32Core0_TEST_I2C_ssd1306_ADS11115_MIDI_PCF8574/ESP32%20Step%20Sequencer%20Schematic.png">

3D-printed box
<img src="https://github.com/ErichHeinemann/ESP32-Audio-Tests-ML/raw/main/ESP32Core0_TEST_I2C_ssd1306_ADS11115_MIDI_PCF8574/IMG_5668.JPG">

LAB-Board Wiree&Wrap from underneth
<img src="https://github.com/ErichHeinemann/ESP32-Audio-Tests-ML/raw/main/ESP32Core0_TEST_I2C_ssd1306_ADS11115_MIDI_PCF8574/IMG_5635.JPG">
