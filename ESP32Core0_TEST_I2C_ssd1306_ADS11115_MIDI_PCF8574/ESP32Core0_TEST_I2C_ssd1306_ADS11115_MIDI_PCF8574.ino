/**************************************************************************

E.Heinemann 2021-06-04

Test of ESP32 with a lot of additional I2C-Components connecteed to SDA/SCL on GPIO 5/23
1. 3x PCF8574 I2C Muxer In/Out
2. 1x ADS1115 Quad ADC
3. 1x SD1306 OLED

Die Kommunikation mit I2C erfolgt komplett über Core 0

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

#define MIDIRX_PIN 16
#define MIDITX_PIN 17

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

#define SAMPLE_RATE  44100

// PCF8574 
// adjust addresses if needed
// AUTHOR of PCF8574-Lib: Rob Tillaart
#include "PCF8574.h"
PCF8574 PCF1( 0x27, &I2Ctwo); // PCF1 - Top Row The addresses were discoverd with a I2C-Scanner-Sketch!
PCF8574 PCF2( 0x23, &I2Ctwo); // PCF2 - Bottom Row
PCF8574 PCF3( 0x20, &I2Ctwo); // PCF3 - Additional 5 vButtons and Rotary Encoder with 3 Pins

// Steps are stored in pcf_value1 and pcf_value2 active-Bit is deactivated step, low bit is activated step? Inverse Logic!!
byte pcf_value1, pcf_value2, pcf_value3;
// old values
byte pcf_value1_1, pcf_value2_1, pcf_value3_1;
// Read-Values of Buttonstate..
byte tmp_pcf_value1_1, tmp_pcf_value2_1, tmp_pcf_value3_1, tmp_pcf_value3_1_but;

float bpm=131.5;

boolean func_but_pressed = false;

// Test for Display
String active_track_name="Kick";
uint8_t active_track_num = 1;
uint16_t active_step = 0; // 96 Steps per full note, // 24 per Quaternote // 12 bei Achtelnoten, 6 bei 16tell ... Stepsequencer arbeitet mti 16tel.


#define LED_PIN 2
// this is used to add a task to core 0
TaskHandle_t  Core0TaskHnd ;


// These values are only used to make an integration with MIDI, additional analog inputs or with an Menü slightly simplier
// The values could be 0 - 127 or floats ... 
uint8_t global_playbackspeed; // The Playbackspeed
uint8_t global_bitcrush;      // is bitcrusher active
uint8_t global_biCutoff;      // Cutoff-Frequency of the filter
uint8_t global_biReso;        // Resonance of the filter

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
  // Sampler_Init();
  // Effect_Init();
  xTaskCreatePinnedToCore( Core0Task, "Core0Task", 8000, NULL, 5, &Core0TaskHnd, 0);
}

/*
inline void audio_task(){
    // prepare out samples for processing
    float fl_sample = 0.0f;
    float fr_sample = 0.0f;

    Sampler_Process( &fl_sample, &fr_sample );
    Effect_Process( &fl_sample, &fr_sample );

    //Don’t block the ISR if the buffer is full
    if (!i2s_write_samples(fl_sample, fr_sample)){
        // error!
    }
}
*/



uint8_t act_menuNum = 0;
uint8_t act_page = 0;
uint8_t act_instr = 1; // Pad1 Changed by Rotary Encodere

// Menu
String   menus[] = { "Instr", "Sound", "Global", "Velo", "Speed", "Bars", "Notes", "Scale", "Sync" };
uint8_t  menuNum[] = { 0, 1, 2, 3 , 4, 5, 6 , 7, 8 };

String act_menu=menus[ act_menuNum ];

// uint8_t pages[] = {0,1,2,3,4,5,6,7,8};
// Structure of "Instrument" must cover all these values

// Instr
char pages[36][8]=
{   'Attack', 'Decay', 'Pitch','Volume'
  , 'Filter', '-', 'MidiNote','-'
  ,  'Filter', 'Reso', 'BitCrush','-'
  , 'Accent', 'Normal', '-','-'
  , 'Main', 'Fine', '-','-'
  , 'Bars', '-', '-','-'
  , 'Scheme', '-', '-','-'
  , 'Scale', '-', '-','-'
  , 'Sync In', 'Sync Out', '-' ,'-'
};

// Is the Sequencer running or not
boolean  playBeats = true;

// Instruments, Accent is not an Instrument but internally handled as an instrument .. therefore 17 Instruments from 0 to 16
const String shortInstr[] ={ "ACC"   , "PAD1", "PAD2" , "PAD 3" , "PAD 4", "PAD 5", "PAD 6", "PAD 7", "PAD 8", "PAD 9", "PAD 10", "PAD 11", "PAD 12","PAD 13","PAD 14","PAD 15","PAD 16" };
const String instrument[]      ={ "Accent", "PAD 1", "PAD 2" , "PAD 3" , "PAD 4", "PAD 5", "PAD 6", "PAD 7", "PAD 8", "PAD 9", "PAD 10", "PAD 11", "PAD 12","PAD 13","PAD 14","PAD 15","PAD 16" };
uint8_t iSound[] ={ -1,  36, 37, 38, 39, 40, 41, 42, 43,  44, 45, 46, 47, 48, 49, 50, 51 }; // MIDI-Sound, edited via Menu 50=TOM, 44=closed HH, 
uint8_t iVelo[]  ={ 127, 90, 90, 90, 90, 90, 90, 90, 90,  90, 90, 90, 90, 90, 90, 90, 90 }; // Velocity, edited via Menu
uint8_t inotes1[]={ 255,255,255,255,255,255,255,255,255, 255,255,255,255,255,255,255, 255 };


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
String param_name0="Attack"; 
String param_name1="Decay"; 
String param_name2="Sustai"; 
String param_name3="Releas"; 

int16_t adc0, adc1, adc2, adc3;
int16_t adc0_1, adc1_1, adc2_1, adc3_1;

uint8_t adc_slope = 15;

boolean do_display_update = false;

float volts0, volts1, volts2, volts3;

uint16_t display_prescaler = 0;
uint16_t midi_prescaler = 0; 
uint16_t ads_prescaler = 0; 
uint16_t patch_edit_prescaler = 0; // Wait seconds after loading to sync the values

uint8_t val0_synced = 0; // 0 adc-value is lower, 2 adc-value is higher, 1 = values are synced
uint8_t val1_synced = 0; // 0 adc-value is lower, 2 adc-value is higher, 1 = values are synced
uint8_t val2_synced = 0; // 0 adc-value is lower, 2 adc-value is higher, 1 = values are synced
uint8_t val3_synced = 0; // 0 adc-value is lower, 2 adc-value is higher, 1 = values are synced

int16_t val_dev_null = 0;

uint8_t compare2values( uint16_t val1, uint16_t val2 ){
  if(val1 < val2 ) return 0;
  if(val1 > val2 ) return 2;
  return 1;
}

void Core0TaskLoop(){
  // put your loop stuff for core0 here
  
  readPCF_rotary_fast();

  ads_prescaler +=1;
  if( ads_prescaler > 20 ){
    readPCF();  // for the 16 Steps
    readPCF3(); // for the Control-Buttons
    ads1115red( 0 , param_val0, param_val1, param_val2, param_val3 );
    ads_prescaler = 0;
  }

  display_prescaler +=1;
  if(( do_display_update== true && display_prescaler > 4 ) || display_prescaler > 150  ){
    update_display_bars();
    display_prescaler = 0;
    do_display_update = false;
  }

  if( patch_edit_prescaler == 25 ){
    val0_synced = compare2values(param_val0, patch_val0 );    
    val1_synced = compare2values(param_val1, patch_val1 );    
    val2_synced = compare2values(param_val2, patch_val2 );    
    val3_synced = compare2values(param_val3, patch_val3 );    
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

void Core0TaskSetup(){
  // I2C
  pinMode( SDA2, INPUT_PULLUP); 
  pinMode( SCL2, INPUT_PULLUP); 
  I2Ctwo.begin(SDA2,SCL2, 50000 ); // SDA2 pin 0, SCL2 pin 5, works with 50.000 Hz if pinned to other Core0

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

void loop(){
  //midi_prescaler += 1;
  //if( midi_prescaler > 5 ){
    if( MIDI.read() ){}
    //midi_prescaler = 0;
  //}

}
