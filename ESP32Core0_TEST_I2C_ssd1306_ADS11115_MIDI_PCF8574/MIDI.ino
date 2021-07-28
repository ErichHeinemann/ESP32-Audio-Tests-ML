byte last_channel=0;
byte last_number=0;
byte last_value=0;

#define DEBUG_MIDI

// midi_channel = 10;

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

float new_pitch = 0.5;

void MIDI_setup(){
  MIDI.begin();
  // Register MIDI-Handlers
  MIDI.setHandleNoteOn( handleNoteOn );
  MIDI.setHandleNoteOff( handleNoteOff );
  MIDI.setHandleControlChange( handleControlChange );
  MIDI.setHandleProgramChange( handleProgramChange );
  MIDI.setHandlePitchBend( handlePitchBend );
  MIDI.setHandleClock( handleClock );
  MIDI.setHandleStart( handleStart );
  MIDI.setHandleStop( handleStop );
  MIDI.setHandleContinue( handleContinue );
  MIDI.setHandleContinue( handleContinue );

}


// MIDI-Handlers
void handleNoteOff(byte channel, byte note, byte velocity){
  if( channel == midi_channel ){ 
#ifdef DEBUG_MIDI
    Serial.println("NoteOff");
#endif
  }  
}

void handleNoteOn(byte channel, byte note, byte velocity){
  if( channel == midi_channel ){
#ifdef DEBUG_MIDI
    Serial.println("NoteOn");
#endif 
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
  }
  
}

void handleAfterTouchPoly(byte channel, byte note, byte pressure){
}




void handleProgramChange(byte channel, byte number){
#ifdef DEBUG_MIDI  
  Serial.printf("ProgramChange %X %X \n", channel, number);  
#endif  
  if( channel == midi_channel ){
    // fill the I2S-Cache because the loading of other samples takes some time!
    for( int i = 0; i <= 512; i++){
      if( !i2s_write_samples( 0.0f, 0.0f ) ){
         // error!
      }
    }
    // Change progNumber
    // Here, we have 2 Programs with differenet samples, 0 and 1 ... 
    Sampler_SetProgram( number );
  }

  
}  

void handleControlChange( byte channel, byte number, byte value){
  if( channel == midi_channel ){
    // My Novation Circuit is sending the same values for a long time. I won´t process them
    if( last_channel == channel && last_number==number && last_value == value ){
      return;  
    }
    last_channel = channel;
    last_number=number;
    last_value=value;
#ifdef DEBUG_MIDI    
    Serial.printf("CC >%02x %02x %02x\n", channel, number, value );
#endif    
    // ADSR Attack
    if( number ==0x50 && patch_val0 != value){
      patch_val0 = value;    
      if( patch_val0>127 ) patch_val0 = 127;
      val0_synced = compare2values(param_val0, patch_val0 );  
    }
    // Decay
    if( number ==0x52 && patch_val1 != value ){
      patch_val1 = value;
      if( patch_val1>127 ) patch_val1 = 0127;
      val1_synced = compare2values(param_val1, patch_val1 );  
    }
    // Sustain
    if( number ==0x54 && patch_val2 != value){
      patch_val2 = value;
      if( patch_val2>127 ) patch_val2 = 0127;
      val2_synced = compare2values(param_val2, patch_val2 );  
    }
    // Release
    if( number ==0x56 && patch_val3 != value){
      patch_val3 = value;
      if( patch_val3>127 ) patch_val3 = 0127;
      val3_synced = compare2values(param_val2, patch_val2 );  
    }  
  } 
}

void handleAfterTouchChannel(byte channel, byte pressure){
}

void handlePitchBend(byte channel, int bend ){
  if( channel == midi_channel ){
#ifdef DEBUG_MIDI    
    Serial.println("PitchBend");
#endif    
  }
}

void handleSystemExclusive(byte* array, unsigned size){
}

void handleTimeCodeQuarterFrame(byte data){
}

void handleSongPosition(unsigned int beats){
}

void handleSongSelect(byte songnumber){
}

void handleTuneRequest(void){
}

void handleClock(void){
    // If an external Clock is sending, switch off the internal Clock and wait for "Start" or "Stop" or "Continue"
    external_clock_in = true;
    sequencer_callback();
#ifdef DEBUG_MIDI  
if( playBeats == true ){
    Serial.println("MIDI Clock");
}
#endif      
}

void handleStart(void){
#ifdef DEBUG_MIDI  
  Serial.println("MIDI Start");
#endif  
  sequencer_start();
}

void handleContinue(void){
#ifdef DEBUG_MIDI  
  Serial.println("MIDI Continue");
#endif
}
void handleStop(void){
#ifdef DEBUG_MIDI  
  Serial.println("MIDI Stop");
#endif
  sequencer_stop();
}

void handleActiveSensing(void ){
}
void handleSystemReset(void ){
#ifdef DEBUG_MIDI  
  Serial.println("MIDI SystemReset");
#endif
}


inline
void MIDI_Process(){
  MIDI.read(); 
}
