/*
 * a simple implementation to use midi
 *
 * Author: Marcel Licence
 */

// #define RXD2 16 /* U2RRXD */
// #define TXD2 17

/* * Formatting strings <fmt>
 * %B    - binary (d = 0b1000001)
 * %b    - binary (d = 1000001)  
 * %c    - character (s = H)
 * %d/%i - integer (d = 65)\
 * %f    - float (f = 123.45)
 * %3f   - float (f = 123.346) three decimal places specified by %3.
 * %o    - boolean on/off (d = On)
 * %s    - char* string (s = Hello)
 * %X    - hexidecimal (d = 0x41)
 * %x    - hexidecimal (d = 41)
 * %%    - escaped percent ("%")
 * Thanks goes to @alw1746 for his %.4f precision enhancement 
 */

MIDI_CREATE_INSTANCE( HardwareSerial, Serial2, MIDI );



/* use define to dump midi data */
#define DEBUG_MIDI


// enable the next line to filter MIDI by Channel 10! If the line is disable, the module will listen on all MIDI-Channels
#define FILTERCHANNEL 10

#define MIDITHROUGHOFF

/* constant to normalize midi value to 0.0 - 1.0f */
#define NORM127MUL	0.007874f


// uint8_t lastnote = 0;
// uint8_t lastpitch = 0;

// I didi not calculate these values, I used a guitar-tuner and tried to find propper values to play some tuned Kickdrums in the range of 3 octaves.
// these values aree not good enough to create a steinway-piano!
float pitches[37] = {
     -0.00025, 0.01938 , 0.0387   , 0.0605 , 0.0799  , 0.10340 ,  0.121 ,  0.1439 , 0.1638 , 0.1828 ,  0.2084   , 0.2278
    , 0.2493 , 0.26961 , 0.288865 , 0.3099 , 0.33201 , 0.352555, 0.374575,  0.3931 , 0.4134 , 0.4351 , 0.45414 , 0.4778
    , 0.4973 , 0.5178  , 0.5395   , 0.5607 , 0.58129 , 0.6021  ,  0.6250 ,  0.6429 , 0.6652 , 0.6879 , 0.70694 , 0.728094
    , 0.749155
   }; 
  //  a      a#          h          c         c#        d          d#          e         f      f#         g         g#                   

//    
#define PITCHUP     1.05946f // muliplicator0. to pitch by one semitone
#define PITCHDOWN   0.943877 // muliplicator to pitch by one semitone down

inline void Midi_Setup(){
    MIDI.begin(MIDI_CHANNEL_OMNI);
    // MIDI_Thru to off to spent some CPU-Time
#ifdef MIDITHROUGHOFF
    MIDI.turnThruOff();
#endif    
    // Define Callbacks
    MIDI.setHandleNoteOn( handleNoteOn );
    MIDI.setHandleNoteOff( handleNoteOff );     
    MIDI.setHandlePitchBend( handlePitchBend );      
    MIDI.setHandleControlChange( handleControlChange );  
    MIDI.setHandleProgramChange( handleProgramChange );
    MIDI.setHandleStart( handleStart );    
    MIDI.setHandleStop( handleStop );    
    MIDI.setHandleClock( handleClock );    
    MIDI.setHandleContinue( handleContinue );      

    Serial.println("Setup done");
}

float new_pitch = 0.5;


void handleNoteOn(byte channel, byte note, byte velocity){
  
#ifdef DEBUG_MIDI
   Serial.println();
   Serial.printf("NoteOn %X %X %X\n", channel, note, velocity);
#endif
#ifdef FILTERCHANNEL
  if( channel == FILTERCHANNEL ){
#endif
  // Ideas for Pitch:
  // - calculate a Pitch based on the played octave
  // - calculate a pitch based on the played velocity
  // - use the lookup-table "pitches" to play a note chromatically
  
   
   /*
   // select this note for the currrent Editor
   // derive a new Pitch based on the played octave
   // get the Octave:
   new_pitch = 0.5;
   // uint8_t octave = floor( note / 12 );
   // Serial.printf("Octave %d %d \n", octave,  note - 45 ); 

   uint8_t mynote = 56; // Note, wo der Kammerton A3 drauf liegt
   if( note >= 44 && note <= 82 ){ 
     new_pitch = (float) pitches[ note - 45 ];
   }

   Sampler_SelectNote( mynote );    
   Sampler_SetSoundPitch( new_pitch );
   */
   // Serial.printf("new_pitch %f \n", new_pitch ); 

switch (note) {
  case 36:
    note = 0;  // Kick
    break;
  case 38:
    note = 1;  // Snare
    break;
  case 43:
    note = 2; // Low Tom
    break;
  case 50:
    note = 3; // High Tom
    break;  
  case 42:
    note = 7; // Closed Hihat
    break;
  case 46:
    note = 6;  // OpenHihat
    break;
  case 39:
    note = 5;  // Clap
    break;
  case 75:
    note = 4; // Rimshot
    break;
  case 67:
    note = 11; // Maracas
    break;      
  case 49:
    note = 9; // Crash
    break;            
  default:
    // Statement(s)
    break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
}


   Sampler_NoteOn( note , velocity );
   
#ifdef FILTERCHANNEL
  }
#endif
}

void handleNoteOff( byte channel, byte note, byte velocity){
  Serial.printf("NoteOff %X %X %X\n", channel, note, velocity);  
}

void handleControlChange( byte channel, byte number, byte value){
  Serial.printf("CC %X %X %X\n", channel, number, value);
  // - with a CC, You could probably control the global Effects Bitcruscher, Global Pitch or the Filter

#ifdef FILTERCHANNEL
  if( channel == FILTERCHANNEL ){
#endif
  // - Sampler_SetPlaybackSpeed( NORM127MUL * value ); // value from 0 to 2
  // Modwheel CC==1  
  if( number ==1 ){
    Effect_SetBiReso( NORM127MUL * value );
  } 

switch( number ){
  case 0x36: // Stutter TIME >> is now Filter Freq - it should better go to an extra POT
    // Effect_SetBiCutoff( NORM127MUL * value );
    break;
  case 0x37: // Stutter DEPTH - it should better go to an extra POT
    // Effect_SetBiReso( NORM127MUL * value );
    break;   
  case 0x32: // PCM Speed von Clap
    Sampler_SelectNote( 5 ) ;  
    Sampler_SetSoundPitch( NORM127MUL * value ); // Sampler_SetPlaybackSpeed( NORM127MUL * value );
    break;    
  case 0x33: // PCM Speed von Claves bzw. RimShot
    Sampler_SelectNote( 4 ) ;  
    Sampler_SetSoundPitch( NORM127MUL * value ); // Sampler_SetPlaybackSpeed( NORM127MUL * value );
    break;      
   case 0x34: // agogo Pitch - here Maracas
    Sampler_SelectNote( 11 ) ;
    Sampler_SetSoundPitch( NORM127MUL * value );
    break;      
  case 0x35: // Crash Pitch
    Sampler_SelectNote( 9 ) ;
    Sampler_SetSoundPitch( NORM127MUL * value );
    break;       
  case 0x38: // Tom Decay
    Sampler_SelectNote( 2 ) ;
    Sampler_SetDecay( 10, 0 , value );
    Sampler_SelectNote( 3 ) ;
    Sampler_SetDecay( 10, 0 , value );
    break;      
  case 0x39: // Closed Decay
    Sampler_SelectNote( 7 ) ;
    Sampler_SetDecay( 10, 0 , value );
    break;    
  case 0x3A: // Closed Decay
    Sampler_SelectNote( 6 ) ;
    Sampler_SetDecay( 10, 0 , value );
    break;  

  case 0x3B: // Hihat Grain
    Effect_SetBitCrusher( NORM127MUL * value );
    break;     
  default:
    // Statement(s)
    break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
}
  
  // - Effect_SetBitCrusher( float value )
  // - Effect_SetBiCutoff(float value )

  // Or You could control the values for a specific Sampleplayer/Note ( select the note via Sampler_SelectNote( uint8_t note ) )
  // - Sampler_SetSoundPitch(float value) // for a selectedNote
  // - Sampler_SetDecay( uint8_t ch, uint8_t data1, uint8_t data2)  // data2 should be the Value from thee CC, channel and data1 could be ignored

#ifdef FILTERCHANNEL
  }
#endif
  
}

void handleProgramChange(byte channel, byte number){
  Serial.printf("ProgramChange %X %X \n", channel, number);  
#ifdef FILTERCHANNEL
  if( channel == FILTERCHANNEL ){
#endif
  
  // fill the I2S-Cache because the loading of other samples takes some time!
  for( int i = 0; i <= 512; i++){
    if( !i2s_write_samples( 0.0f, 0.0f ) ){
       // error!
    }
  }
  
  // Change progNumber
  // Here, we have 2 Programs with differenet samples, 0 and 1 ... 
  Sampler_SetProgram( number );

#ifdef FILTERCHANNEL
  }
#endif
  
}  

void handleClock(void){
    // the Volca is sending Clock nonstop Serial.println("O Clock");  
}

void handlePitchBend( byte channel, int bend ){
  
  // Pitchbend is not a CC! and it is 14Bit!
  Serial.printf("Pitchbend %X %d\n", channel, bend);    
  // The Bend-Value goes from -8192 to 8191 
  // Normalize the Bend to a Value from 0 to 1 to use it in the sketch for effects; 
  float newBend = NORM127MUL * NORM127MUL * (bend + 8192) / 1.015744f; // 
  Serial.printf("Pitchbend %X %3f\n", channel, newBend );
#ifdef FILTERCHANNEL
  if( channel == FILTERCHANNEL ){
#endif  
  // - with the bend, You could probably control the Bitcruscher, Global Pitch or the Filter
  // - Sampler_SetPlaybackSpeed( newBend ); // Cool effect!!! value from 0 to 1
  // - Effect_SetBiReso( newBend ); // switched from Lowpass to Highpass, cool, in combination with the Mod-Wheel
  // - Effect_SetBitCrusher( float value )
  Effect_SetBiCutoff( newBend ); // switched from Lowpass to Highpass, cool, in combination with the Mod-Wheel

  // Or someting if pitch is negativ and something else if the bend is positiv ... 
#ifdef FILTERCHANNEL
  }
#endif
}

void handleStart(void){
      Serial.println("> Start");  
}

void handleContinue(void){
      Serial.println("> Continue");  
}

void handleStop(void){
      Serial.println("# Stop");  
}

inline
void MIDI_Process(){
    MIDI.read(); 
}
  
