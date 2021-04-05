/*
 * this file is the main project file which can be opened and compiled with arduino
 *
 * Author: Marcel Licence
 */

 /* 
  *  Modified Version from Erich Heinemann 2021-04-05
  *  - removed the Sequencer to simplify the Arduino-Sketch
  *  - replaced custom MIDI-Code by the standard Arduino MIDI-Library from FourtysevenEffects
  *  - changeed some lines of the sampler and midi_interface to be able to switch between 2 directories with samples via program-change
  */

#include <Arduino.h>
#include "FS.h"
#include <LITTLEFS.h>
#include <MIDI.h>

#include <WiFi.h>

#define SAMPLE_RATE	44100

volatile uint8_t midi_prescaler = 0;


// #define ANALOG_INPUTS  // If this is defined, the system will use analog inputs of ADC 1 and ADC2 to react on inputs in the range of 0 - 2.5Volts!!


#ifdef ANALOG_INPUTS
volatile uint8_t analog_prescaler = 0;
uint8_t gpio_playbackspeed = 32; // Good GPIO-Ports on ADC1 with 12 Bit Resolution, max input is 2.5Volts. To use them with 5 Volts, please use 2 or more resistors to split the level. and protect the inputs from getting more than 3.3volts!!
uint8_t gpio_bitcrush =      33;
uint8_t gpio_biCutoff =      34;
uint8_t gpio_biReso   =      35;

uint8_t tmp_playbackspeed; // The Playbackspeed
uint8_t tmp_bitcrush;      // is bitcrusher active
uint8_t tmp_biCutoff;      // Cutoff-Frequency of the filter
uint8_t tmp_biReso;        // Resonance of the filter

uint8_t last_playbackspeed; // The Playbackspeed
uint8_t last_bitcrush;      // is bitcrusher active
uint8_t last_biCutoff;      // Cutoff-Frequency of the filter
uint8_t last_biReso;        // Resonance of the filter
#endif

// These values are only used to make an integration with MIDI, additional analog inputs or with an Menü slightly simplier
// The values could be 0 - 127 or floats ... 
uint8_t global_playbackspeed; // The Playbackspeed
uint8_t global_bitcrush;      // is bitcrusher active
uint8_t global_biCutoff;      // Cutoff-Frequency of the filter
uint8_t global_biReso;        // Resonance of the filter


// With the ESP32 perhaps You would use analog Inputs to control all the things or use Touch-Inputs to Trigger and Control the sounds
// It is up to You

// In this version, the settings are not stored!
  
void setup(){
    // put your setup code here, to run once:
    delay(500);

    Serial.begin(115200);
    Serial.println();
    Serial.printf("Loading data\n");
    Serial.printf("Firmware started successfully\n");

    Blink_Setup();
    setup_i2s();
    Midi_Setup();

#if 0
    setup_wifi();
#else
    WiFi.mode(WIFI_OFF);
#endif

    btStop();

    Sampler_Init();
    Effect_Init();


    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());
}


inline void audio_task(){
    /* prepare out samples for processing */
    float fl_sample = 0.0f;
    float fr_sample = 0.0f;

    Sampler_Process( &fl_sample, &fr_sample );
    Effect_Process( &fl_sample, &fr_sample );


    //Don’t block the ISR if the buffer is full
    if (!i2s_write_samples(fl_sample, fr_sample)){
        // error!
    }
}


inline
void loop_1Hz(void){
    Blink_Process();
}


void loop(){

    audio_task();
    static uint32_t loop_cnt;

    loop_cnt ++;
    if( loop_cnt >= SAMPLE_RATE ){
        loop_cnt = 0;
        loop_1Hz();
    }

    midi_prescaler++;
    if( midi_prescaler >= 8 ){
        MIDI_Process();
        midi_prescaler = 0;
    }
    
#ifdef ANALOG_INPUTS
    // The valuees could be changed by Potentiometers or by MIDI
    analog_prescaler++;
    if( analog_prescaler > 23 ){
     tmp_playbackspeed = floor( analogRead( gpio_playbackspeed ) /32 );
     tmp_bitcrush = floor( analogRead( gpio_bitcrush ) /32 );
     tmp_biCutoff = floor( analogRead( gpio_biCutoff ) /32 );
     tmp_biReso = floor( analogRead( gpio_biReso ) /32 );
               
     if( tmp_playbackspeed !=  last_playbackspeed ){
        last_playbackspeed = tmp_playbackspeed;
        global_playbackspeed = tmp_playbackspeed;
        Sampler_SetPlaybackSpeed( NORM127MUL * global_playbackspeed );
     }  
     if( tmp_bitcrush !=  last_bitcrush ){
        last_bitcrush = tmp_bitcrush;
        global_bitcrush = tmp_bitcrush;
        Effect_SetBitCrusher( NORM127MUL * global_bitcrush )
     }  
     if( tmp_biCutoff !=  last_biCutoff ){
        last_biCutoff = tmp_biCutoff;
        global_biCutoff = tmp_biCutoff;
        Effect_SetBiCutoff( NORM127MUL * global_biCutoff )
     }  
     if( tmp_biReso !=  last_biReso ){
        last_biReso = tmp_biReso;
        global_biReso = tmp_biReso;
        Effect_SetBiReso( NORM127MUL * global_biReso );
     }

    // ADD here your favorite things
    // trigger samples via buttons or control-voltagee or do something else here.
    // request data from touch-sensors?

    
     
     analog_prescaler = 0;
    }
    
#endif

    
}
