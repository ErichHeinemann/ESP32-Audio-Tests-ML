// 2021-07-05 E.Heinemann, this file covers all functions of the sequencer. probably I have to create the sequencer and tracks as classes - perhaps as a structure.


// Scale, Menu to change Scale is yet not implemented
uint8_t velocity = 90;
uint8_t scale=1 ;//Default scale  1/16
uint8_t      scale_value[]  = {     3,      6,    12,     24,       8,       16 };
const String scale_string[] = { "1/32", "1/16", "1/8", "1/4",  "1/8T",   "1/4T" };


float sequencer_cal_delay( float new_bpm ){
  float tempo_delay = 60000000 / new_bpm / 24;  
  return tempo_delay;
}

void sequencer_start(){
  //if( playBeats == false && buttonStateL != oldStateL && buttonStateL == LOW ){
  // Start-Button
  active_step=0;
  playBeats = true;
  // lcd.setCursor(0,1);
  // lcd.print( "Started Beating " );
  // MIDI-Clock
  MIDI.sendRealTime( MIDI_NAMESPACE::Start );
  //}
}

void sequencer_stop(){
  // Stop-Button was pressed or MIDI-Stop received
  active_step=0;
  playBeats = false;
  // lcd.setCursor(0,1);
  // lcd.print( "Stopped Beating ");
  MIDI.sendRealTime(MIDI_NAMESPACE::Stop);
}

void sequencer_continue(){
    
}





// ####
// #### Show Notes and current Step via LEDs on 1. and 2. PCF8574 ######
// ####
void showStep ( int mystep, uint8_t address1, uint8_t address2, uint8_t notes1, uint8_t notes2 ){
  uint8_t bitdp1 = notes1;
  uint8_t bitdp2 = notes2;
  // Current Step would be only shown if played
  if( playBeats==true ){ 
    if( mystep <  8 ){ bitClear( bitdp1, active_step ); }
    if( mystep >= 8 ){ bitClear( bitdp2, ( active_step -8 ) ); }
  }

 // sollte besser eine Funktion im Modull PCF sein!!
  PCF1.write8( bitdp1 );
  PCF2.write8( bitdp2 );
  
  //WriteIo( bitdp1, address1 );
  //WriteIo( bitdp2, address2 );
}


// ####
// #### Play the Midi-Notes of thee act step ##### 
// ####
void Update_Midi() {
  // first 8 beats
  if( playBeats==true ){
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
           MIDI.sendNoteOff( iSound[i],         0, midi_channel );       
           MIDI.sendNoteOn(  iSound[i], velocity ,midi_channel );
         }  
       }
       

       /*
       // ARP
       for( int i = 1; i < 4; i++ ){
         if( bitRead( inotes1[i], step_position ) == 0 ){
           velocity  = round( iVelo[i] * veloAccent / 100);
           if( velocity > 127 ){
             velocity = 127; 
           }

           if( arp_in_notes_list[i] > 32 ){
             midiA.sendNoteOff( arp_in_notes_list[i],         0, midi_channel_arp1 );     
             midiA.sendNoteOn(  arp_in_notes_list[i],  velocity, midi_channel_arp1 );
           }          
           // midiA.sendNoteOff( iSound[i],         0, midi_channel );       
           // midiA.sendNoteOn(  iSound[i], velocity ,midi_channel );
           
         }  
       }
       */
       
    }
    
    // second Byte or second 8 Steps
    // bitClear(bitdp2, (a-8));
    if( active_step >= 8 ){ 
       // play Notes2
      if (bitRead( inotes2[0], active_step -8 ) == 0){ // Accent is set
        veloAccent = iVelo[0];
      }
      
      // loop through all instruments .. but ignore the Accent with 0

      for( int i = 1; i < count_instr; i++ ){
        if( bitRead( inotes2[i], ( active_step -8 ) ) == 0 ){
          velocity  = round(iVelo[i] * veloAccent / 100);
          if( velocity > 127 ){
            velocity = 127; 
          }
          MIDI.sendNoteOff( iSound[i],         0, midi_channel );       
          MIDI.sendNoteOn(  iSound[i], velocity , midi_channel );
        }  
      }   
      /*
       // ARP-Instruments
       for( int i = 1; i <= 4; i++ ){
         if( bitRead( inotes2[i], step_position-8 ) == 0 ){
           velocity  = round( iVelo[i] * veloAccent / 100);
           if( velocity > 127 ){
             velocity = 127; 
           }

           if( arp_in_notes_list[i] > 32 ){
             midiA.sendNoteOff( arp_in_notes_list[i],         0, midi_channel_arp1 );     
             midiA.sendNoteOn(  arp_in_notes_list[i],  velocity, midi_channel_arp1 );
           }          
           // midiA.sendNoteOff( iSound[i],         0, midi_channel );       
           // midiA.sendNoteOn(  iSound[i], velocity ,midi_channel );
           
         }  
       }
       */
       
    }
    // if we do it with more than 16 STeps , they have to be defined here

    
  }
}


// ####
// #### // Callback from Timer1
// ####
void sequencer_callback(){ 
  if( old_bpm != bpm ){
    // Timer1 is missing on EESP32  !!
    // TODO
    // Timer1.initialize(tempo_delay); old_bpm = bpm;
  }
  count_ppqn++;    
  if( count_ppqn >= scale_value[scale] ){ 
    active_step++;
    if( active_step >= count_bars ){
      active_step = 0;
    }
    Update_Midi();
    count_ppqn =0;
    // Clock Sync out for analog deevices or LED
    // digitalWrite(13, digitalRead(13) ^ 1);
  }     

  // Update Display
  // TODO
  // lcd.setCursor(0,1);
  // lcd.print ("Step:" + String(step_position) + " " + scale_value[scale] );

        
}


