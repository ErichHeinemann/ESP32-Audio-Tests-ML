/**************************************************************************


E.Heinemann 2021-06-04
Test of ESP32 with a lot of additional I2C-Components connecteed to SDA/SCL on GPIO 5/23
1. 3x PCF8574 I2C Muxer In/Out , PCF1&2 drive the LEDs and Step-Buttons, third PCF drives Control-Buttons and Rotary-Encoder
2. 1x ADS1115 Quad ADC
3. 1x SD1306 OLED
Die Kommunikation mit I2C erfolgt komplett über Core 0

2021-07-26 E.Heinemann
Midi Sync, ESP32 ist slave


Buttons:

Encoder - Yes - Reeturn
Menü - Page Start Stop


1x PCM5102 Hifi DAC

MIDI-Out/in, connected to GPIO 16/17

One LED on Pin 2

**************************************************************************/

#include <Arduino.h>
#include "FS.h"
#include <LITTLEFS.h>

#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// MIDI-Library FoutysevenEffects
#include <MIDI.h>
#define NORM127MUL  0.007874f

#define MIDIRX_PIN 16
#define MIDITX_PIN 17


// #define DEBUG_MAIN

struct Serial2MIDISettings : public midi::DefaultSettings{
  static const long BaudRate = 31250;
  static const int8_t RxPin  = MIDIRX_PIN;
  static const int8_t TxPin  = MIDITX_PIN; // LED-Pin
};

HardwareSerial MIDISerial(2);

MIDI_CREATE_CUSTOM_INSTANCE( HardwareSerial, MIDISerial, MIDI, Serial2MIDISettings );

// ADS1115 - Integration
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;  // Use this for the 16-bit version

// remove resistors 70 and 68  on AI Thinker Audio 2.2 - Board to free up Pin 5 and Pin 23!

#define SDA2 5
#define SCL2 23

// #define SDA1 33
// #define SCL1 32

// TwoWire I2Cone = TwoWire(0);
TwoWire I2Ctwo = TwoWire(0);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // The Address was discovered using the I2C Scanner
Adafruit_SSD1306 display( SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET );

#define ADC_ADDRESS 0x48

#define SAMPLE_RATE  38200

// PCF8574 
// adjust addresses if needed
// AUTHOR of PCF8574-Lib: Rob Tillaart
#include "PCF8574.h"
PCF8574 PCF1( 0x27, &I2Ctwo); // PCF1 - Top Row The addresses were discoverd with a I2C-Scanner-Sketch!
PCF8574 PCF2( 0x23, &I2Ctwo); // PCF2 - Bottom Row
PCF8574 PCF3( 0x20, &I2Ctwo); // PCF3 - Additional 5 vButtons and Rotary Encoder with 3 Pins

byte step_pattern_1=255, step_pattern_2=255; // unmodified pattern - stored for the instrument...

// Steps are stored in pcf_value1 and pcf_value2 active-Bit is deactivated step, low bit is a activated step! Inverse Logic!!
byte pcf_value1, pcf_value2, pcf_value3;

// Values which are used to display
// old-values
byte pcf_value1_1, pcf_value2_1, pcf_value3_1;

// Read-Values of Buttonstate..
byte tmp_pcf_value1_1, tmp_pcf_value2_1, tmp_pcf_value3_1, tmp_pcf_value3_1_but;

byte tmp_pcf_value3 ;

boolean  external_clock_in = false;
float    bpm = 90.5;
float    old_bpm = 90.5;
uint8_t  bpm_pot_midi =(int) ( bpm - 30 ) / 2; // range 30 <-> 285 bpm mit MIDI_Values 0-127
uint8_t  bpm_pot_midi_old = bpm_pot_midi;

uint8_t  bpm_pot_fine_midi =  (bpm  - ( bpm_pot_midi*2 + 30 ))*20 + 65; // range ist zu testen ...
uint8_t  bpm_pot_fine_old_midi = bpm_pot_fine_midi;

uint16_t count_ppqn = 0;
uint8_t  veloAccent = 120;
uint8_t  midi_channel = 10;
uint8_t  count_bars = 16;

boolean  func1_but_pressed = false; // left blue
boolean  func2_but_pressed = false; // upper blue for global menus
boolean  func3_but_pressed = false; // 


// Test for Display
String   active_track_name="Kick";
uint8_t  active_track_num = 1;
uint16_t active_step = 0; // 96 Steps per full note, // 24 per Quaternote // 12 bei Achtelnoten, 6 bei 16tell ... Stepsequencer arbeitet mit 16tel Noten, daher 6 Clock-Signale von einem Step zum nächsten.


#define LED_PIN 2
// this is used to add a task to core 0
TaskHandle_t  Core0TaskHnd ;


// These values are only used to make an integration with MIDI, additional analog inputs or with an Menü slightly simplier
// The values could be 0 - 127 or floats ... 
uint8_t global_playbackspeed; // The Playbackspeed
uint8_t global_bitcrush;      // is bitcrusher active
uint8_t global_biCutoff;      // Cutoff-Frequency of the filter
uint8_t global_biReso;        // Resonance of the filter

uint8_t global_playbackspeed_midi; // The Playbackspeed
uint8_t global_bitcrush_midi;      // is bitcrusher active
uint8_t global_biCutoff_midi;      // Cutoff-Frequency of the filter
uint8_t global_biReso_midi;        // Resonance of the filter

uint8_t global_playbackspeed_midi_old; // The Playbackspeed
uint8_t global_bitcrush_midi_old;      // is bitcrusher active
uint8_t global_biCutoff_midi_old;      // Cutoff-Frequency of the filter
uint8_t global_biReso_midi_old;        // Resonance of the filter

uint8_t act_menuNum = 0;
uint8_t act_page = 0;
uint8_t act_instr = 1; // Pad1 Changed by Rotary Encodere

// Menu
String   menus[] = { "Instr", "Sound", "Global", "Velo", "Speed", "Bars", "Notes", "Scale", "Sync" };
uint8_t  menuNum[] = { 0,        1,       2,        3 ,     4,       5,       6 ,    7,       8 };

String act_menu=menus[ act_menuNum ];

uint8_t act_menuNum_max = 8;

// Structure of "Instrument" must cover all these values

// Instr
String pages[] = { "Volume", "Decay", "Pitch","Pan"
  , "MidiNote", "-", "-","-"
  , "Filter", "Reso", "BitCrush","PitDec"
  , "Accent", "Normal", "-","-"
  , "Main", "Fine", "-","Pitch"
  , "Bars", "-", "-","-"
  , "Scheme", "-", "-","-"
  , "Scale", "-", "-","-"
  , "Sync In", "Sync Out", "-", "-"};

String no_display = "-";  

// Is the Sequencer running or not
boolean  playBeats = false;

// Instruments, Accent is not an Instrument but internally handled as an instrument .. therefore 17 Instruments from 0 to 16
// const 
String shortInstr[] ={ "ACC"   , "LCO", "SN" , "HHcl" , "HHop", "Cr", "Cl", "LT", "HT", "S1", "S2", "S3", "S4"
                       ,"T1","T2","T3","T4" };
const String instrument[]      ={ "Accent", "S1", "S2" , "S3" , "S4", "S5", "S6", "S7", "S8", "PAD 9", "PAD 10", "PAD 11", "PAD 12","PAD 13","PAD 14","PAD 15","PAD 16" };
uint8_t iSound[] ={ -1,  36, 37, 38, 39, 40, 41, 42, 43,  44, 45, 46, 47, 48, 49, 50, 51 }; // MIDI-Sound, edited via Menu 50=TOM, 44=closed HH, 
uint8_t iVelo[]  ={ 127, 90, 90, 90, 90, 90, 90, 90, 90,  90, 90, 90, 90, 90, 90, 90, 90 }; // Velocity, edited via Menu
uint8_t inotes1[]={ 255, 255,255,255,255,255,255,255,255, 255,255,255,255,255,255,255, 255 };
uint8_t inotes2[]={ 255, 255,255,255,255,255,255,255,255, 255,255,255,255,255,255,255, 255 };
boolean is_muted[]={ false, false,false,false,false ,false,false,false,false ,false,false,false,false 
                    ,false,false,false,false };
                    
uint8_t volume_midi[] = { 127, 127,127,127,127, 127,127,127,127, 127,127,127,127, 127,127,127,127 };
uint8_t decay_midi[] = { 127, 127,127,127,127, 127,127,127,127, 127,127,127,127, 127,127,127,127  };
uint8_t pitch_midi[] = { 64, 64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64 };
uint8_t pan_midi[]   = { 64, 64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64 };

float step_delay[] ={ 10.1,10.1,10.1,10.1, 10.1,10.1,10.1,10.1, 10.1,10.1,10.1,10.1, 10.1,10.1,10.1,10.1  ,10.1 };
uint8_t count_instr = 17;

// We get some values for parameters from a Patchmanager
// param_valX is the current value
uint8_t param_val0, param_val1, param_val2, param_val3;
// patch_val0 is the value from the patchmanager
// uint8_t patch_val0, patch_val1, patch_val2, patch_val3;
// Some Values for the Patch .. only for testing
uint8_t patch_val0 = 100;
uint8_t patch_val1 = 50;
uint8_t patch_val2 = 25;
uint8_t patch_val3 = 10;


// Some Names for the values - only for testing
String param_name0="Volume  "; 
String param_name1="Decay   "; 
String param_name2="Pitch   "; 
String param_name3="EG      "; 

int16_t adc0, adc1, adc2, adc3;
int16_t adc0_1, adc1_1, adc2_1, adc3_1;

uint8_t adc_slope = 15;

boolean do_display_update = false;
boolean do_display_update_fast = false;

float volts0, volts1, volts2, volts3;

uint16_t display_prescaler = 0;
uint16_t display_prescaler_fast = 0;
uint16_t midi_prescaler = 0; 
uint16_t ads_prescaler = 0; 
uint16_t patch_edit_prescaler = 0; // Wait seconds after loading to sync the values

uint8_t val0_synced = 0; // 0 adc-value is lower, 2 adc-value is higher, 1 = values are synced
uint8_t val1_synced = 0; // 0 adc-value is lower, 2 adc-value is higher, 1 = values are synced
uint8_t val2_synced = 0; // 0 adc-value is lower, 2 adc-value is higher, 1 = values are synced
uint8_t val3_synced = 0; // 0 adc-value is lower, 2 adc-value is higher, 1 = values are synced

int16_t val_dev_null = 0;


static uint32_t loop_cnt;





uint8_t compare2values( uint16_t val1, uint16_t val2 ){
  if(val1 < val2 ) return 0;
  if(val1 > val2 ) return 2;
  return 1;
}

// ####
// ####   SETUP    ####
// ####
void setup(){
  // LED-Pin to check frequently activities only needed for testing
  pinMode( LED_PIN, OUTPUT ); 

  // Serial
  Serial.begin( 115200 );
  Serial.println( __FILE__ );

  // MIDI on Core 1 - the default-Core for Arduino
  pinMode( MIDIRX_PIN , INPUT_PULLUP);  // 22: GPIO 22, u2_RXD 
  //Serial2.begin(31250, SERIAL_8N1, 22, 19);
  // midiA.begin( MIDI_CHANNEL_OMNI );
  MIDISerial.begin( 31250, SERIAL_8N1, MIDIRX_PIN, MIDITX_PIN ); // midi port
  MIDI_setup();

  setup_i2s();

#if 0
    setup_wifi();
#else
    WiFi.mode(WIFI_OFF);
#endif
  btStop();
  Sampler_Init();
  Effect_Init();
  xTaskCreatePinnedToCore( Core0Task, "Core0Task", 8000, NULL, 5, &Core0TaskHnd, 0);

  sequencer_new_instr( act_instr );

#ifdef DEBUG_MAIN
  Serial.printf( "ESP.getFreeHeap() %d\n", ESP.getFreeHeap() );
  Serial.printf( "ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap() );
  Serial.printf( "ESP.getHeapSize() %d\n", ESP.getHeapSize() );
  Serial.printf( "ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap() );
#endif
  
}


long myTimer = millis();
long myTimer_bar = millis();
long cur_time = millis();
long myTimer_Delta = sequencer_calc_delay( bpm );

// ###
// ### Audio-Task ###
// ###
inline void audio_task(){
    /* prepare out samples for processing */
    float fl_sample = 0.0f;
    float fr_sample = 0.0f;

    Sampler_Process( &fl_sample, &fr_sample );
    Effect_Process( &fl_sample, &fr_sample );

    //Don’t block the ISR if the buffer is full
    if( !i2s_write_samples(fl_sample, fr_sample )){
        // error!
    }
}

// ###
// ### Core 0 Loop ###
// ###
int16_t act_instr_tmp =  act_instr;
void Core0TaskLoop(){
  // put your loop stuff for core0 here

  // 0 Instrument
  if( act_menuNum == 0 ){
    // Select new Instrument
    act_instr_tmp =  readPCF_rotary_fast( act_instr );
    if( act_instr_tmp != act_instr ){
      if(  act_instr_tmp<0 ){
        act_instr_tmp = 16; 
      }
      if( act_instr_tmp>16 ){
        act_instr_tmp = 0; // ACC
      }
      // store old Instrument-Beats
      // TODO
      sequencer_new_instr( act_instr_tmp );
    }
  }

  // 1 Sound
  if( act_menuNum == 1 ){
    uint16_t tmp_x =  readPCF_rotary_fast( 1 *2 );
  }  
  // 2 Global
  if( act_menuNum == 2 ){
    uint16_t tmp_x =  readPCF_rotary_fast( 1 *2 );
  } 
  // 3 Velo
  if( act_menuNum == 3 ){
    uint16_t tmp_x =  readPCF_rotary_fast( 1 *2 );
  } 
  // 4 Speed
  if( act_menuNum == 4 ){
    // Change Speed by Buttons or Rotary
    old_bpm = bpm;
    uint16_t tmp_bpm =  readPCF_rotary_fast( bpm *2 );
    bpm = 0.5f * tmp_bpm;
    if( bpm != old_bpm ){
      if( bpm < 30.0f ){
        bpm = 30.0f;
        // sequencer_new_instr( act_instr_tmp );
      }
      if( bpm > 285.0f ){
        bpm = 285.0f;
        // sequencer_new_instr( act_instr_tmp );
      }
      myTimer_Delta = sequencer_calc_delay( bpm );
    }
  }
  // 5 Bars
  if( act_menuNum == 5 ){
    uint16_t tmp_x =  readPCF_rotary_fast( 1 *2 );
  } 
  // 6 Notes
  if( act_menuNum == 6 ){
    uint16_t tmp_x =  readPCF_rotary_fast( 1 *2 );
  }
  // 7 Scale
  if( act_menuNum == 7 ){
    uint16_t tmp_x =  readPCF_rotary_fast( 1 *2 );
  }
  // 8 Sync
  if( act_menuNum == 8 ){
    uint16_t tmp_x =  readPCF_rotary_fast( 1 *2 );
  }

  ads_prescaler +=1;
  if( ads_prescaler > 30 ){
    readPCF();  // for the 16 Steps
    readPCF3(); // for the Control-Buttons
    ads1115red( 0 , param_val0, param_val1, param_val2, param_val3 );
    ads_prescaler = 0;
  }

  display_prescaler +=1;
  if(( do_display_update== true && display_prescaler > 6 ) || display_prescaler > 170  ){
    update_display_bars();
    display_prescaler = 0;
    do_display_update = false;
    do_display_update_fast = false;
  }

  if( playBeats==true ){
    //display_prescaler_fast +=1;
    //if( display_prescaler_fast> 6 && do_display_update_fast== true ){
      pcf_update_leds();
     // do_display_update_fast = false;
     // display_prescaler_fast = 0;
    //}
  }

  if( patch_edit_prescaler == 25 ){
    val0_synced = compare2values( param_val0, patch_val0 );    
    val1_synced = compare2values( param_val1, patch_val1 );    
    val2_synced = compare2values( param_val2, patch_val2 );    
    val3_synced = compare2values( param_val3, patch_val3 );    
  }
  
  if( patch_edit_prescaler > 50 && (  val0_synced != 1 || val1_synced != 1 || val2_synced != 1 || val3_synced != 1 ) ){
    if( val0_synced != 1 && compare2values(param_val0, patch_val0 ) != val0_synced ) val0_synced = 1;
    if( val1_synced != 1 && compare2values(param_val1, patch_val1 ) != val1_synced ) val1_synced = 1;
    if( val2_synced != 1 && compare2values(param_val2, patch_val2 ) != val2_synced ) val2_synced = 1;
    if( val3_synced != 1 && compare2values(param_val3, patch_val3 ) != val3_synced ) val3_synced = 1;
  }else{
    patch_edit_prescaler +=1;  
  }
  
}

// ###
// ### Core 0 Setup ###
// ### used for hardware-UI
// ###
void Core0TaskSetup(){
  // I2C
  pinMode( SDA2, INPUT_PULLUP); 
  pinMode( SCL2, INPUT_PULLUP); 
  I2Ctwo.begin(SDA2,SCL2, 200000 ); // SDA2 pin 0, SCL2 pin 5, works with 50.000 Hz if pinned to other Core0

  // SSD1306 OLED - Display first to be able to show Errors on the Display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin( SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // PCF8574
  PCF_setup();

  // ADS1115
  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");
  ads.begin( ADC_ADDRESS,  &I2Ctwo   );
  Serial.println("ADS1115 started");
}

void Core0Task(void *parameter){
    Core0TaskSetup();
    while( true ){
        Core0TaskLoop();
        // this seems necessary to trigger the watchdog
        delay(1);
        yield();
    }
}



// ###
// ### Default - Loop ###
// ### Audio, Sequencer, MIDI
// ###
void loop(){
  audio_task();
  loop_cnt ++;
  if( loop_cnt >= SAMPLE_RATE ){
      loop_cnt = 0;
      // loop_1Hz();
      // Was soll man in diesem Loop noch machen?
  }

  // Stepsequencer
  cur_time =  millis();
  if( external_clock_in == false ){

    if( playBeats && myTimer_Delta + myTimer < cur_time ){  
      myTimer_bar = myTimer_Delta + myTimer;
      sequencer_callback();
#ifdef DEBUG_MAIN    
      Serial.print("Time-Difference Clock:");
      Serial.println( cur_time - ( myTimer_Delta + myTimer ) ); 
#endif       
      myTimer = myTimer + myTimer_Delta;
    }

    
/*
    
    // only for MIDI-Clock, the for step 0 and not that precise
    if( playBeats && ( count_ppqn > 0 || ( count_ppqn==0 && active_step==0 ))  && myTimer_Delta + myTimer < cur_time ){  
      if( active_step==0 && count_ppqn==0 ){
        //myTimer_bar = myTimer_bar + step_delay[ 16 ];// myTimer_Delta + myTimer;
        myTimer_bar = myTimer_Delta + myTimer;
      } 
      sequencer_callback();
#ifdef DEBUG_MAIN    
      Serial.print("Time-Difference Clock:");
      Serial.println( cur_time - ( myTimer_Delta + myTimer ) ); 
#endif       
      myTimer = myTimer + myTimer_Delta;
    }
    
    // Triggering the Sounds and a bit more precise
    if( playBeats && count_ppqn == 0 && active_step > 0 && myTimer_bar + step_delay[ active_step ] < cur_time ){  
      myTimer = myTimer_bar + step_delay[ active_step ] + myTimer_Delta;
      sequencer_callback();
#ifdef DEBUG_MAIN    
      Serial.print("Time-Difference Precise:");
      Serial.println( cur_time - ( myTimer_Delta + myTimer ) ); 
#endif       
    }  
*/
    
  }

  
  // MIDI  
  midi_prescaler++;
  if( midi_prescaler >= 2 ){
      MIDI_Process();
      midi_prescaler = 0;
  }

}
