// 2021-07-05 E.Heinemann, this file covers all functions of the sequencer. probably I have to create the sequencer and tracks as classes - perhaps as a structure.

// #define DEBUG_SEQUENCER
// #define DEBUG_SEQUENCER_TIMING


// Scale, Menu to change Scale is yet not implemented
uint8_t velocity = 90;
uint8_t scale=1 ;//Default scale  1/16
uint8_t scale_value[]       = {     2,      5,    12,     24,       8,       16 };
const String scale_string[] = { "1/32", "1/16", "1/8", "1/4",  "1/8T",   "1/4T" };

// toggles Mute
void sequencer_mute_instr(){
  //  act_instr
  is_muted[ act_instr ] = ! is_muted[ act_instr ];
}

// toggles Solo
void sequencer_solo_instr(){
  //  act_instr
}

void sequencer_new_instr( uint8_t new_instr_num  ){

  // store the values of the last instrument
  inotes1[ act_instr ] = step_pattern_1;
  inotes2[ act_instr ] = step_pattern_2;
  //Serial.print("old_step_pattern_1: " );
  //Serial.println( step_pattern_1 );
  
  // get the stored values of the new instrument  
  act_instr = new_instr_num;
  step_pattern_1 = inotes1[ act_instr ];
  step_pattern_2 = inotes2[ act_instr ];

  //Serial.print("new_step_pattern_1: ");
  //Serial.println( step_pattern_1 );

  // Poti-Values synchronisieren
  // Decay
  param_val1 = Sampler_GetSoundDecay_Midi();
  // Pitch 
  param_val2 = Sampler_GetSoundPitch_Midi();
  // Panorama
  param_val3 = Sampler_GetSoundPan_Midi();
  
  pcf_value1 = step_pattern_1;
  pcf_value2 = step_pattern_2;
  active_track_num = act_instr;
  active_track_name = shortInstr[ act_instr ];
#ifdef DEBUG_SEQUENCER 
  Serial.print("new Instrument: " );
  Serial.print( active_track_num );
  Serial.print( " " );
  Serial.println( active_track_name );
#endif  
  do_display_update = true;
  
}

float sequencer_calc_delay( float new_bpm ){
  // Delay for count_ppqn and a Array for the Step-Timer
  float tempo_delay = 60000 / ( new_bpm * scale_value[scale] * 4 ); // scale-value provides the count of Clock-Ticks per 4th note ..  normally 16th equals 6 steps * 4 to get   

  // bpm_pot_midi =(int) ( new_bpm -30 ) / 2; // range 30 <-> 285 bpm mit MIDI_Values 0-127
  // bpm_pot_fine_midi = new_bpm - ( ( bpm_pot_midi*2 ) +30 );
  // bpm_pot_fine_midi = ( new_bpm  - ( bpm_pot_midi * 2 + 30 ) ) * 20 + 65;
  
  // calculate the delay for every step of the 16 steps
  // TODO: This is the right place to insert some kind of "Swing"
  for( int i=0; i <= 16; i++ ){
    step_delay[i] = i * tempo_delay * scale_value[scale];
#ifdef DEBUG_SEQUENCER_TIMING    
    Serial.print("step_delay: ");
    Serial.print( i );
    Serial.print( " " );
    Serial.println( step_delay[i] );
#endif
  }
  
#ifdef DEBUG_SEQUENCER    
  Serial.print("tempo_delay: ");
  Serial.println( tempo_delay );
#endif  

  return tempo_delay;
}

// #### 
// #### Start Sequencer
// ####
void sequencer_start(){
  active_step = 0; // 0 -- 15
  count_ppqn = 0;
  if( external_clock_in == false ){
    myTimer_Delta = sequencer_calc_delay( bpm );
    myTimer = millis() - myTimer_Delta;
    myTimer_bar = myTimer;
  }
#ifdef DEBUG_SEQUENCER
  Serial.println("Start");
#endif
  // MIDI-Clock
  playBeats = true;
  MIDI.sendRealTime( MIDI_NAMESPACE::Start );
}


// #### 
// #### Stop Sequencer
// ####
void sequencer_stop(){
  // Stop-Button was pressed or MIDI-Stop received
  // Der Step darf nicht auf 0 gesetzt werden, denn es könnte ein Continue erfolgen
  // active_step=0;
  playBeats = false;
#ifdef DEBUG_SEQUENCER
  Serial.println("Stop");
#endif
  // lcd.setCursor(0,1);
  // lcd.print( "Stopped Beating ");
  MIDI.sendRealTime(MIDI_NAMESPACE::Stop);
}

// #### 
// #### Continue Sequencer
// ####
void sequencer_continue(){
  if( playBeats == false ){
    myTimer_Delta = sequencer_calc_delay( bpm );
    myTimer = millis() - myTimer_Delta;
    myTimer_bar = myTimer; // is wrong and must be calculated based on the current step and ppqn!
    playBeats = true;
#ifdef DEBUG_SEQUENCER
    Serial.println("Continue");
#endif
    // MIDI-Clock
    // MIDI.sendRealTime( MIDI_NAMESPACE::Start ); 
  }
}


// ####
// #### Show Notes and current Step via LEDs on 1. and 2. PCF8574 ######
// ####
void showStep( uint8_t notes1, uint8_t notes2 ){
  uint8_t bitdp1 = notes1;
  uint8_t bitdp2 = notes2;
  // Current Step would be only shown if played
  if( playBeats==true ){ 
    if( active_step <  8 ){ bitClear( bitdp1, active_step ); }
    if( active_step >= 8 ){ bitClear( bitdp2, ( active_step -8 ) ); }
  }
  // sollte besser eine Funktion im Modull PCF sein!!
  PCF1.write8( bitdp1 );
  PCF2.write8( bitdp2 );
}

// ####
// #### Update the Notes of the current Instrument ##### 
// ####
void sequencer_update_track( byte pcf_value1_1, byte pcf_value2_1 ){
  inotes1[ act_instr ] =  pcf_value1_1;
  inotes2[ act_instr ] =  pcf_value2_1;
}


// ####
// #### Play the Midi-Notes of the actual active_step ##### 
// ####
void sequence_process(){
  // first 8 beats
  if( playBeats==true ){
       
#ifdef DEBUG_SEQUENCER    
    Serial.println("sequence_process");
#endif    
    veloAccent = 100; // Normal Velocity by default
    // first Byte or first 8 Hits
    // "song_position" is the current step 
    if( active_step<8 ){ // play notes1
      // Accent set?
      if( bitRead( inotes1[0], active_step ) == 0 ){
        // Accent is set
        veloAccent = iVelo[0];
      } 
      

      // loop through all instruments .. But ignore Accent with 0       
      for( int i = 1; i < count_instr; i++ ){
        if( bitRead( inotes1[i], active_step ) == 0 ){
          velocity  = round( iVelo[i] * veloAccent / 100);
          if( velocity > 127 ) {
            velocity = 127; 
          }
          MIDI.sendNoteOff( iSound[i],         0, midi_channel ); // MIDI-Off could be removed if You trigger external analog gear. For Samplers you should keep it     
          MIDI.sendNoteOn(  iSound[i], velocity ,midi_channel );  // iSound is the array with the MIDI-Note-Number for the Pads
          Sampler_NoteOn( i-1, velocity ); // Accent "0" has no Sample to play
#ifdef DEBUG_SEQUENCER 
          Serial.println("NoteOn");
#endif
         }  
       }
       
    }
    
    // second Byte or second 8 Steps
    // bitClear(bitdp2, (a-8));
    if( active_step >= 8 ){ 
       // play Notes2
      if( bitRead( inotes2[0], active_step -8 ) == 0 ){ // Accent is set
        veloAccent = iVelo[0];
      }
      // loop through all instruments .. but ignore the Accent with 0
      for( int i = 1; i < count_instr; i++ ){
        if( bitRead( inotes2[i], ( active_step -8 ) ) == 0 ){
          velocity  = round(iVelo[i] * veloAccent / 100);
          if( velocity > 127 ){
            velocity = 127;
          }
          MIDI.sendNoteOff( iSound[i],         0, midi_channel ); // MIDI-Off could be removed if You trigger external analog gear. For Samplers you should keep it       
          MIDI.sendNoteOn(  iSound[i], velocity , midi_channel ); // iSound is the array with the MIDI-Note-Number for the Pads
          Sampler_NoteOn( i-1, velocity ); // Accent "0" has no Sample to play
#ifdef DEBUG_SEQUENCER 
          Serial.println("NoteOn");
#endif          
        }
      }
    }
    // if we do it with more than 16 Steps , they have to be defined here    
  } 
}

// ####
// #### Callback from Timer1
// ####
void sequencer_callback(){
  if( playBeats == true ){
    if( external_clock_in == false && old_bpm != bpm ){
      // Timer1 is missing on ESP32  !!
      myTimer_Delta = sequencer_calc_delay( bpm );
      myTimer = millis() - myTimer_Delta;
      myTimer_bar = myTimer;
      old_bpm = bpm;
    }
      
    if( count_ppqn ==0 ){
      sequence_process();
      digitalWrite( LED_PIN, HIGH ); 
  #ifdef DEBUG_SEQUENCER  
      Serial.print("sequencer_callback() "); 
      Serial.print( count_ppqn ); 
      Serial.print( " " ); 
      Serial.println( active_step );
  #endif
    }
    
    if( count_ppqn >= scale_value[scale] ){
      active_step++;
      if( active_step >= count_bars ){
        active_step = 0;
      }
      count_ppqn =0;
  
      // Clock Sync out for analog devices or LED
      // digitalWrite(13, digitalRead(13) ^ 1);
  
      // showStep( pcf_value1, pcf_value2 );
      // pcf_update_leds();
      do_display_update_fast = true;
    }else{
      count_ppqn++;  
      digitalWrite( LED_PIN, LOW );
    } 
  }
}
