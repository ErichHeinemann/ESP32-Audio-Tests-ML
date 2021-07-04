byte last_channel=0;
byte last_number=0;
byte last_value=0;

#define MYCHANNEL 1

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
  Serial.println("NoteOff");
}

void handleNoteOn(byte channel, byte note, byte velocity){
  Serial.println("NoteOn");
}

void handleAfterTouchPoly(byte channel, byte note, byte pressure){
}

void handleControlChange(byte channel, byte number, byte value){

  // My Novation Circuit is sending the same values for a long time. I won´t process them
  if( last_channel == channel && last_number==number && last_value == value ){
    return;  
  }
  last_channel = channel;
  last_number=number;
  last_value=value;
  
  Serial.printf("CC >%02x %02x %02x\n", channel, number, value );
  // ADSR Attack
  if( channel==MYCHANNEL && number ==0x50 && patch_val0 != value){
    patch_val0 = value;    
    if( patch_val0>127 ) patch_val0 = 127;
    val0_synced = compare2values(param_val0, patch_val0 );  
  }
  // Decay
  if( channel==MYCHANNEL && number ==0x52 && patch_val1 != value ){
    patch_val1 = value;
    if( patch_val1>127 ) patch_val1 = 0127;
    val1_synced = compare2values(param_val1, patch_val1 );  
  }
  // Sustain
  if( channel==MYCHANNEL && number ==0x54 && patch_val2 != value){
    patch_val2 = value;
    if( patch_val2>127 ) patch_val2 = 0127;
    val2_synced = compare2values(param_val2, patch_val2 );  
  }
  // Release
  if( channel==MYCHANNEL && number ==0x56 && patch_val3 != value){
    patch_val3 = value;
    if( patch_val3>127 ) patch_val3 = 0127;
    val3_synced = compare2values(param_val2, patch_val2 );  
  }   
}

void handleProgramChange(byte channel, byte number){
  Serial.println("ProgramChange");
}

void handleAfterTouchChannel(byte channel, byte pressure){
}

void handlePitchBend(byte channel, int bend ){
  Serial.println("PitchBend");
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
}

void handleStart(void){
  Serial.println("MIDI Start");
}

void handleContinue(void){
  Serial.println("MIDI Continue");
}
void handleStop(void){
  Serial.println("MIDI Stop");
}

void handleActiveSensing(void ){
}
void handleSystemReset(void ){
  Serial.println("MIDI SystemReset");
}
