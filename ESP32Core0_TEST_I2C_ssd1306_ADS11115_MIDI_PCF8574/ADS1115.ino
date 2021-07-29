// The user could navigate through different Menus and could select different instruments
// Finally, there are about 16x4 + n Values to manage, but only 4 pots.
// The pots are "virtual" connected to the current menu / instrument
// If the user changes the menu, the current value is displayed and if the pot is not at the same value, 
// then 2 bars are shown, one for the value from RAM/Patch and one value from the Pot. if the Value of the Pot crosses 
// the value of the Patch, it will be converted into one bar.
// E.Heinemann 2021-06-04


void ads1115red( int show_serial , uint8_t & param_val0 , uint8_t & param_val1 , uint8_t & param_val2 , uint8_t & param_val3  ){
  // store old value in a variable
  adc0_1 = adc0;
  adc1_1 = adc1;
  adc2_1 = adc2;
  adc3_1 = adc3;

  // fetch new values with a slightly lowpass and better precision
  adc0 = ( adc0*1 + ads.readADC_SingleEnded(0)  ) / 2;
  // delay(1);
  adc1 = ( adc1*1 + ads.readADC_SingleEnded(1)  ) / 2;
  // delay(1);
  adc2 = ( adc2*1 + ads.readADC_SingleEnded(2)  ) / 2;
  // delay(1);
  adc3 = ( adc3*1 + ads.readADC_SingleEnded(3)  ) / 2;

/* Better:
  // fetch new values with a slightly lowpass and better precision
  adc0 = ( adc0*2 + ads.readADC_SingleEnded(0) + ads.readADC_SingleEnded(0) ) / 4;
  // delay(1);
  adc1 = ( adc1*2 + ads.readADC_SingleEnded(1) + ads.readADC_SingleEnded(1) ) / 4;
  // delay(1);
  adc2 = ( adc2*2 + ads.readADC_SingleEnded(2) + ads.readADC_SingleEnded(2) ) / 4;
  // delay(1);
  adc3 = ( adc3*2 + ads.readADC_SingleEnded(3) + ads.readADC_SingleEnded(3) ) / 4;
*/

/*
  if( do_display_update == false )&& ( adc0 > adc0_1 +adc_slope || adc0<adc0_1 -adc_slope   ) ){
    do_display_update = true;    
    param_val0 = map( adc0, 0, 17600, 0, 127 );
  }
  if( do_display_update == false && ( adc1 > adc1_1 +adc_slope || adc1<adc1_1 - adc_slope  ) ){
    do_display_update = true;    
    param_val1 = map( adc1, 0, 17600, 0, 127 );
  }
  
  if( do_display_update == false && ( adc2 > adc2_1 +adc_slope || adc2<adc2_1 -adc_slope   ) ){
    do_display_update = true;    
    param_val2 = map( adc2, 0, 17600, 0, 127 );

    if( act_menuNum == 0 && act_instr > 0 ){
      // Change Pitch
      pitch_midi[ act_instr-1 ] = param_val2;
      Serial.print("New Pitch");
      Serial.println( param_val2 );
      // Sampler_SetSoundPitch_Midi( param_val2 );
    }
    
  }
  if( do_display_update == false && ( adc3 > adc3_1 +adc_slope || adc3<adc3_1 -adc_slope   ) ){
    do_display_update = true;    
    param_val3 = map( adc3, 0, 17600, 0, 127 );
  }

  if( do_display_update == true ){
    param_val0 = map( adc0, 0, 17600, 0, 127 );
    param_val1 = map( adc1, 0, 17600, 0, 127 );
    param_val2 = map( adc2, 0, 17600, 0, 127 );
    param_val3 = map( adc3, 0, 17600, 0, 127 );
  }
 */

  if(  adc0 > adc0_1 +adc_slope || adc0<adc0_1 -adc_slope ){
    do_display_update = true;    
    param_val0 = map( adc0, 0, 17600, 0, 127 );

    if( act_menuNum == 0 && act_instr > 0 && val0_synced == 1 ){
      // Change Pitch
      volume_midi[ act_instr ] = param_val0;
      Serial.print("New Volume");
      Serial.println( param_val0 );
    } 

    if( act_menuNum == 1 && act_instr > 0 && val0_synced == 1 ){
      // Change MidiNote
      midinote_in_out[ act_instr ] = param_val0;
    } 


    if( act_menuNum == 2 && val0_synced == 1 ){
      // Menu Global Effects Change Filter Frequency
      global_biCutoff_midi = param_val0;
    }

    if( act_menuNum == 4 && val0_synced == 1 ){
      // Change Main-BPM
      bpm_pot_midi = param_val0;
      bpm = (float) 30.0f + bpm_pot_midi*2 + ( bpm_pot_fine_midi-65 )/20.0f;
      myTimer_Delta = sequencer_calc_delay( bpm );
      Serial.print("New Speed Main");
      Serial.println( param_val0 );
    } 

  }

  
  if( adc1 > adc1_1 +adc_slope || adc1<adc1_1 - adc_slope ){
    do_display_update = true;    
    param_val1 = map( adc1, 0, 17600, 0, 127 );

    if( act_menuNum == 0 && act_instr > 0 && val1_synced == 1 ){
      // Change Decay
      decay_midi[ act_instr ] = param_val1;
    }

    if( act_menuNum == 1 && act_instr > 0 && val1_synced == 1 ){
      // Change MidiChannel Out
      param_val1 = map( adc1, 0, 17600, 1, 16 );      
      midichannel_in_out[ act_instr ] = param_val1;
    } 

    if( act_menuNum == 2 && val1_synced == 1 ){
      // Menu Global Effects Change Filter Resonance
      global_biReso_midi = param_val1;
    }

    if( act_menuNum == 4 && val1_synced == 1){
      // Change Main-BPM
      bpm_pot_fine_midi = param_val1;
      bpm = (float) 30.0f + bpm_pot_midi*2 + ( bpm_pot_fine_midi-65 )/20.0f;
      myTimer_Delta = sequencer_calc_delay( bpm );
    } 

    
  }
  
  if( adc2 > adc2_1 +adc_slope || adc2<adc2_1 -adc_slope ){
    do_display_update = true;    
    param_val2 = map( adc2, 0, 17600, 0, 127 );

    if( act_menuNum == 0 && act_instr > 0 && val2_synced == 1){
      // Change Pitch
      pitch_midi[ act_instr ] = param_val2;
    }

    if( act_menuNum == 1 && act_instr > 0 && val2_synced == 1){
      // Menu Instr Change Attack
      attack_midi[ act_instr ] = param_val2;
    }    

    if( act_menuNum == 2  && val2_synced == 1){
      // Menu Global Effects Change Bitcrush
      global_bitcrush_midi = param_val2;
    }
    
  }

  
  if( adc3 > adc3_1 +adc_slope || adc3<adc3_1 -adc_slope ){
    do_display_update = true;    
    param_val3 = map( adc3, 0, 17600, 0, 127 );

    if( act_menuNum == 0 && act_instr > 0 && val3_synced == 1){
      // Menu Instr Change Pitch
      pan_midi[ act_instr ] = param_val3;
    }
    
    if( act_menuNum == 1 && act_instr > 0 && val3_synced == 1){
      // Menu Instr Change PitchDecay-MOD (FM-Pitch)
      pitchdecay_midi[ act_instr ] = param_val3;
    }
    
    if( act_menuNum == 2  && val3_synced == 1){
      // Menu Global Effects Change Pitch-Decay-Mod, sounds like FM-Drums
      global_playbackspeed_midi = param_val3;
      if( global_playbackspeed_midi != global_playbackspeed_midi_old ){
        global_playbackspeed_midi_old = global_playbackspeed_midi;
        Sampler_SetPlaybackSpeed_Midi( global_playbackspeed_midi );
      }
    }

    
  }

  if( show_serial > 0 ){
    volts0 = ads.computeVolts(adc0);
    volts1 = ads.computeVolts(adc1);
    volts2 = ads.computeVolts(adc2);
    volts3 = ads.computeVolts(adc3);
    Serial.print("AIN0: "); Serial.print(adc0); Serial.print("  "); Serial.print(volts0); Serial.println("V");
    Serial.print("AIN1: "); Serial.print(adc1); Serial.print("  "); Serial.print(volts1); Serial.println("V");
    Serial.print("AIN2: "); Serial.print(adc2); Serial.print("  "); Serial.print(volts2); Serial.println("V");
    Serial.print("AIN3: "); Serial.print(adc3); Serial.print("  "); Serial.print(volts3); Serial.println("V");
  }
  
}
