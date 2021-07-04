This is a Test-Project in Development

I would like to create a UI for an electronic Instrument based on ESP32 only using I2C for a Display (SSD1306) 16 Step-Buttons, 16 Leds, some Control-Buttons and 4 Potentiometers and finally a Rotary Encoder. The Rotary-Encoder - a standard from the nineties ist more like  a nightmare on I2C. I use only Core0 for the I2C! That way, Core 1 - the default for Arduino is free for Your application! Core 0 supports I2C only with 50KHz - which is less then expected. To get all these GPIO or GPI - Ports, I am using 3x PCF8574 I2C multiplexers which could toggle between Input-muxer and Output-muxer to drive the LEDs. The Rotary-Encoder is connecteed to the last 3 Pins of the third PCF7574.  The code is optimized to request the status of the third PCF8574 as often as possible to get any turn of the encoder. In the current code it is still not optimal.
I will add some screenshots to get an overview how it looks alike in reality.
I named my project, a 16 Stepsequencer with an Sample-Engine "Polca" a combination of the "PO"-Series and the "Volca" - Series.

Why only using I2C?? I try to move to the AI Thinker Audio 2.2 -board and this has less free GPIOs. I was happy to get I2C on this board by removing 2 Resistors!

Final statement:
This code is not for a final product, its purpose is only for testing the capabilities with I2C on Core 0 on ESP32.

Does it manage Midi or create any Sounds: No, not in the current state.



Lab-Board on 3d-printed holder base, the PCF8574 DIL are under the OLED
Testing the UI with the OLED - 16 Steps as bubbles
<img src="/ErichHeinemann/ESP32-Audio-Tests-ML/raw/main/ESP32Core0_TEST_I2C_ssd1306_ADS11115_MIDI_PCF8574/IMG_5640.JPG">
Testing 4 Valuees with 4 Pots (ADS1115)
<img src="/ErichHeinemann/ESP32-Audio-Tests-ML/raw/main/ESP32Core0_TEST_I2C_ssd1306_ADS11115_MIDI_PCF8574/IMG_5634.JPG">

LAB-Board Wiree&Wrap from underneth
<img src="/ErichHeinemann/ESP32-Audio-Tests-ML/raw/main/ESP32Core0_TEST_I2C_ssd1306_ADS11115_MIDI_PCF8574/IMG_5635.JPG">
