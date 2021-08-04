// The user could navigate through different Menus and could select different instruments
// Finally, there are about 16x4 + n Values to manage, but only 4 pots.
// The pots are "virtual" connected to the current menu / instrument
// If the user changes the menu, the current value is displayed and if the pot is not at the same value, 
// then 2 bars are shown, one for the value from RAM/Patch and one value from the Pot. if the Value of the Pot crosses 
// the value of the Patch, it will be converted into one bar.
// E.Heinemann 2021-06-04

void ads1115read_first( ){
  adc0 = ads.readADC_SingleEnded(0);
  delay(1);
  adc1 = ads.readADC_SingleEnded(1);
  delay(1);
  adc2 = ads.readADC_SingleEnded(2);
  delay(1);
  adc3 = ads.readADC_SingleEnded(3);
}  

  
void ads1115read( int show_serial , uint16_t & param_val0 , uint16_t & param_val1 , uint16_t & param_val2 , uint16_t & param_val3  ){
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

  if( adc0 > adc0_1 +adc_slope || adc0<adc0_1 -adc_slope ){
    param_val0 = map( adc0, 0, 17600, 0, 127 );
    if( val0_synced == 1 ){
      switch( act_menuNum ){
        case 0:        
          if( act_instr > 0  ){
            volume_midi[ act_instr ] = param_val0; // Change Pitch
          }else{
            veloAccent_midi = param_val0;
          }
          break;
        case 1:
          if( act_instr > 0 ){
            midinote_in_out[ act_instr ] = param_val0;  // Change MidiNote
          } 
          break;
        case 2:
            global_biCutoff_midi = param_val0; // Menu Global Effects Change Filter Frequency
          break;
        case 3:
          // Soundset
          program_midi = param_val0;
          program_tmp = map( program_midi,0,127,0,countPrograms-1 ); // 5 Sets
          break;
        case 4:
          // Change Main-BPM
          bpm_pot_midi = param_val0;
          bpm = (float) 30.0f + bpm_pot_midi*2 + ( bpm_pot_fine_midi-65 )/20.0f;
          myTimer_Delta = sequencer_calc_delay( bpm );
          break;
        default:
          break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
      }
    }
    do_display_update = true;      
  }

  
  if( adc1 > adc1_1 +adc_slope || adc1<adc1_1 - adc_slope ){
    param_val1 = map( adc1, 0, 17600, 0, 127 );
    if( val1_synced == 1 ){
      switch( act_menuNum ){
        case 0:
          if( act_instr > 0  ){
            decay_midi[ act_instr ] = param_val1; // Change Decay
          }else{
            veloInstr_midi =  param_val1; 
          }  
          break;
        case 1:
          if(  act_instr > 0 ){
            param_val1 = map( adc1, 0, 17600, 1, 16 ); // Change MidiChannel Out
            midichannel_in_out[ act_instr ] = param_val1;
          }
          break;  
        case 2:
          global_biReso_midi = param_val1; // Menu Global Effects Change Filter Resonance
          break;
        case 3:
          // Soundset
          break;  
        case 4:
          // Change Main-BPM
          bpm_pot_fine_midi = param_val1;
          bpm = (float) 30.0f + bpm_pot_midi*2 + ( bpm_pot_fine_midi-65 )/20.0f;
          myTimer_Delta = sequencer_calc_delay( bpm );
          break;
        default:
          break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
      }
    }  
    do_display_update = true; 
  }
  
 
  if( adc2 > adc2_1 +adc_slope || adc2<adc2_1 -adc_slope ){
    param_val2 = map( adc2, 0, 17600, 0, 127 );
    if( val2_synced == 1 ){
      switch( act_menuNum ){
        case 0:
          if( act_instr > 0 ){
            pitch_midi[ act_instr ] = param_val2;  // Change Pitch
          }
          break;
        case 1:
          if(  act_instr > 0 ){
            attack_midi[ act_instr ] = param_val2; // Menu Instr Change Attack
          }    
          break;
        case 2:
          global_bitcrush_midi = param_val2;
          break;
        case 3:
        
          break;
        case 4:
          count_bars_midi= param_val2;
          count_bars = round( param_val2 / 16 );
          break;
        default:
          break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
      }
    }  
    do_display_update = true; 
  }
 
  if( adc3 > adc3_1 +adc_slope || adc3<adc3_1 -adc_slope ){ 
    param_val3 = map( adc3, 0, 17600, 0, 127 );
    if( val3_synced == 1 ){
      switch( act_menuNum ){
        case 0:
          if( act_instr > 0 ){
            // Menu Instr Change Pitch
            pan_midi[ act_instr ] = param_val3;
          }
          break;
        case 1:
          if( act_instr > 0 ){
            // Menu Instr Change PitchDecay-MOD (FM-Pitch)
            pitchdecay_midi[ act_instr ] = param_val3;
          }
          break;
        case 2:
          global_playbackspeed_midi = param_val3;
          if( global_playbackspeed_midi != global_playbackspeed_midi_old ){
            global_playbackspeed_midi_old = global_playbackspeed_midi;
            Sampler_SetPlaybackSpeed_Midi( global_playbackspeed_midi );
          }
          break;
        case 3:
        
          break;
        case 4:
        
          break;
        default:
        
          break;
      }
    }
    do_display_update = true; 
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
