
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

/* constant to normalize midi value to 0.0 - 1.0f */
#define NORM127MUL  0.007874f
#define ADC_TOLERANCE 2.0f

#define ANALOG_INPUTS  // If this is defined, the system will use analog inputs of ADC 1 and ADC2 to react on inputs in the range of 0 - 2.5Volts!!


#ifdef ANALOG_INPUTS
volatile uint16_t analog_prescaler = 0;
uint8_t gpio_playbackspeed = 33; // Good GPIO-Ports on ADC1 with 12 Bit Resolution, max input is 2.5Volts. To use them with 5 Volts, please use 2 or more resistors to split the level. and protect the inputs from getting more than 3.3volts!!
// uint8_t gpio_bitcrush =   33;
uint8_t gpio_biCutoff =      34;
uint8_t gpio_biReso   =  35;
uint8_t gio_pgm_change = 32;
 
uint8_t tmp_pgm_change = 0; // Old Value sof the Button

uint16_t tmp_playbackspeed; // The Playbackspeed
float    tmp_playbackspeed2 = 64.0f;
// uint8_t tmp_bitcrush;      // is bitcrusher active
uint16_t tmp_biCutoff;      // Cutoff-Frequency of the filter
float tmp_biCutoff2 = 64.0f;
uint16_t tmp_biReso;        // Resonance of the filter
float tmp_biReso2 = 64.0f;

uint8_t last_playbackspeed; // The Playbackspeed
// uint8_t last_bitcrush;      // is bitcrusher active
uint8_t last_biCutoff;      // Cutoff-Frequency of the filter
uint8_t last_biReso;        // Resonance of the filter
uint8_t last_pgm = 0;           // last Program     
uint8_t last_pgm_change = 0;
#endif

uint16_t pot_mess1;
uint16_t pot_mess2;
uint16_t pot_mess3;
uint8_t pot_number = 0; // we check the Pots one after another ... 

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

#ifdef ANALOG_INPUTS
  // Pots
  pinMode( gpio_playbackspeed, INPUT_PULLDOWN);
  // Bitcrush is mapped to a CC (HAT Grain)
  // pinMode( gpio_bitcrush, INPUT_PULLDOWN);
  pinMode( gpio_biCutoff, INPUT_PULLDOWN);
  pinMode( gpio_biReso, INPUT_PULLDOWN);   
  //Button     
  pinMode( gio_pgm_change,  INPUT_PULLDOWN );
#endif  

    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());
}



//Code implemented courtesy of Cine-lights via GitHub. Example usage: Filters Microphone Raw data to fit acceptable ranges
float fscale(float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve){
  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  bool invFlag = 0; //Invert Flag

  if( curve > 10 )
  curve = 10;
 
  if( curve < -10 )
  curve = -10;

  curve = curve * (-.1);
  curve = pow(10, curve);

  if( inputValue < originalMin ){
    inputValue = originalMin;
  }
  if( inputValue > originalMax ){
    inputValue = originalMax;
  } 

  //Zero reference the values
  OriginalRange = originalMax - originalMin;
  if( newEnd > newBegin ){
    NewRange = newEnd - newBegin;
  }else{
    NewRange = newBegin - newEnd;
    invFlag = 1; //Invert Flag
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal = zeroRefCurVal / OriginalRange; //Normalize to 0-1 float

  //Check for originalMin > orignalMax
  if( originalMin > originalMax ){
    return 0;
  }

  if( invFlag == 0 ){
    rangedValue = (pow(normalizedCurVal, curve) * NewRange) + newBegin; 
  }else{ //invert range
    rangedValue = newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }
  
  return rangedValue;
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
    if( analog_prescaler > 400 ){

      tmp_pgm_change = digitalRead( gio_pgm_change );
      if( tmp_pgm_change != last_pgm_change ){
        if( tmp_pgm_change == HIGH ){
          last_pgm = last_pgm + 1;
          if( last_pgm > 5 ) last_pgm = 0;
          handleProgramChange( 10 , last_pgm);
        }
      }
      
    if( pot_number == 0){
      pot_mess1 = analogRead( gpio_playbackspeed );
      pot_mess2 = analogRead( gpio_playbackspeed );
      pot_mess3 = analogRead( gpio_playbackspeed );
      if(pot_mess3 < 20){
        pot_mess2 = 0; pot_mess3 = 0;
      }          
      tmp_playbackspeed = ( (tmp_playbackspeed*4 + pot_mess2 + pot_mess3 *2 ) / 7.0f); // changes are more slightly 
      tmp_playbackspeed2 =  fscale( 0, 4095, 5, 120, tmp_playbackspeed, 4.7 );  // makes the log pot into a linear one
      // tmp_playbackspeed = floor( analogRead( gpio_playbackspeed ) /32 ); // first attempt which created nonstop changes...    
 
     if( tmp_playbackspeed2 > ( last_playbackspeed + ADC_TOLERANCE) || tmp_playbackspeed2 <  (last_playbackspeed - ADC_TOLERANCE ) ){
        last_playbackspeed = (int)tmp_playbackspeed2;
        global_playbackspeed = tmp_playbackspeed2;
        // Serial.println( tmp_playbackspeed2 );   
        Sampler_SetPlaybackSpeed( NORM127MUL * global_playbackspeed );
     } 
    }

      
    //  tmp_bitcrush = floor( analogRead( gpio_bitcrush ) /32 );
    // if( tmp_bitcrush !=  last_bitcrush ){
    //   last_bitcrush = tmp_bitcrush;
    //    global_bitcrush = tmp_bitcrush;
    //    Effect_SetBitCrusher( NORM127MUL * global_bitcrush )
    // }

    if( pot_number == 1){
      pot_mess1 = analogRead( gpio_biCutoff ); // only to calibrate the ADC
      pot_mess2 = analogRead( gpio_biCutoff );
      pot_mess3 = analogRead( gpio_biCutoff );      
      if(pot_mess3 < 20){
        pot_mess2 = 0; pot_mess3 = 0;
      }
      tmp_biCutoff = ( (tmp_biCutoff*4 + pot_mess2 + pot_mess3 *2 ) / 7.0f); // changes are more slightly 
      tmp_biCutoff2 =  fscale( 0, 4095, 0, 127, tmp_biCutoff, 4.5 );  // cahnges the log Pot into a  linear Poteentiometer!
      
      // tmp_biCutoff = floor(( pot_mess1 + pot_mess2 )/64 );   
      // tmp_biCutoff = floor( analogRead( gpio_biCutoff ) /32 );
      if( tmp_biCutoff2 > last_biCutoff + ADC_TOLERANCE || tmp_biCutoff2 < last_biCutoff - ADC_TOLERANCE ){
      
          last_biCutoff = tmp_biCutoff2;
          global_biCutoff = tmp_biCutoff2;
          Effect_SetBiCutoff( NORM127MUL * global_biCutoff );
      }  
    }
    
    if( pot_number == 2){
      pot_mess1 = analogRead( gpio_biReso );
      pot_mess2 = analogRead( gpio_biReso );
      pot_mess3 = analogRead( gpio_biReso );    
      if(pot_mess3 < 20){
        pot_mess2 = 0; pot_mess3 = 0;
      }        
      tmp_biReso  =  (tmp_biReso*4 + pot_mess2 + pot_mess3 *3 ) / 7.0f; 
      tmp_biReso2 =  tmp_biReso/33;   // it was already a lin pot
      // fscale( 0, 4095, 0, 127, tmp_biReso, 0.5 );  // makes it into a lineares Potentiometer!

      
      if( tmp_biReso2 > last_biReso + ADC_TOLERANCE || tmp_biReso2 <  last_biReso - ADC_TOLERANCE){
        last_biReso   = (int) tmp_biReso2;
        global_biReso = tmp_biReso2;
        Effect_SetBiReso( NORM127MUL * global_biReso );
      }
    }
    // ADD here your favorite things
    // trigger samples via buttons or control-voltagee or do something else here.
    // request data from touch-sensors?

    
     pot_number = pot_number +1;
     if( pot_number >= 3){
       pot_number = 0; 
     }
     analog_prescaler = 0;
    }
    
#endif

    
}
