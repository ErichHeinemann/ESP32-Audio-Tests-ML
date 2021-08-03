// Menufunction
// Changes im Menu finden im Kontext des aktuellen Menus statt

// History:
// 2021-07-05 E.Heinemann, Added this Menu-File to implement the structure for the menu, step by step.
//                        The Button underneeth the Rotary Encoder acts as a FUNCTION-Button to select something together with one of the 16 STEP-Buttons. On a Akai MPC1000/2500 this button is called Mode-Button
// 2021-07-29 E.Heinemann, Factory and Random Patterns implemented
// 2021-08-03 E.Heinemann, changed Menu-Structure


// 
void change_pattern(){
    // Select a  Default-Pattern 
    current_pattern = current_pattern+1;
    if( current_pattern > 8 ){
      current_pattern = 0;
    }
   
    if( act_instr<=5 ){
      step_pattern_1 = patterns_onbeat[ current_pattern ];
      if( current_pattern < 6 ){
        step_pattern_2 = patterns_onbeat[ current_pattern ];
      }else{
        step_pattern_2 = ((step_pattern_1 & 0xf) << 4) | ((step_pattern_1 & 0xf0) >> 4); // Swap Nibbles to get more
      }  
      pcf_value1 = step_pattern_1;
      pcf_value2 = step_pattern_2;
      inotes1[ act_instr ] = step_pattern_1;
      inotes2[ act_instr ] = step_pattern_2;                 
    }
    
    if( act_instr>5 ){
      step_pattern_1 = patterns_offbeat[ current_pattern ];
      step_pattern_2 = ((step_pattern_1 & 0xf) << 4) | ((step_pattern_1 & 0xf0) >> 4); // Swap Nibbles to get more
      pcf_value1 = step_pattern_1;
      pcf_value2 = step_pattern_2;          
      inotes1[ act_instr ] = step_pattern_1;
      inotes2[ act_instr ] = step_pattern_2;           
    }
    
    Serial.print( "  New-Pattern: ");
    Serial.println( current_pattern );
    return;
}

void random_pattern(){
    // Select a  random Number or copy an exisitng pattern and swap it

// search for an random existing Instrument
    uint8_t random_instr = random( 1, 8 );
    if( random_instr  == act_instr ){
      step_pattern_1 = ((step_pattern_1 & 0xf) << 4) | ((step_pattern_1 & 0xf0) >> 4); // Swap Nibbles to get more
      step_pattern_2 = ((step_pattern_2 & 0xf) << 4) | ((step_pattern_2 & 0xf0) >> 4); // Swap Nibbles to get more
      pcf_value1 = step_pattern_1;
      pcf_value2 = step_pattern_2;
      inotes1[ act_instr ] = step_pattern_1;
      inotes2[ act_instr ] = step_pattern_2;  
      return;
    }
    uint8_t old_notes1 = inotes1[ act_instr ];
    uint8_t old_notes2 = inotes2[ act_instr ];
    if( inotes1[ random_instr ] != 255 ){
      step_pattern_1 = inotes1[ random_instr ];
      inotes1[ random_instr ] = old_notes1;
    }else{
      step_pattern_1 = random( 0, 254 );
    }
    if( inotes2[ random_instr ] != 255 ){
      step_pattern_2 = inotes2[ random_instr ];
      inotes2[ random_instr ] = old_notes2;
    }else{
      step_pattern_2 = random( 0, 254 );     
    }
    pcf_value1 = step_pattern_1;
    pcf_value2 = step_pattern_2;
    inotes1[ act_instr ] = step_pattern_1;
    inotes2[ act_instr ] = step_pattern_2;                 
    Serial.println( "  Random-Pattern: ");
    return;
}


void func1_but( uint8_t step_number ){
  Serial.print( "  act menu: ");
  Serial.print( act_menuNum);
  Serial.print( "  func_but:");
  Serial.println( step_number );

  // Instrument-Menu
  if( act_menuNum == 0 ){
    
    if( step_number < 12 ){
      // select a Instrument 
      sequencer_new_instr( step_number  ); // this is a function on Core 1 !?
      if( act_instr==0 ){
        param_name0 = pages_accent_short[ 0 ];
        param_name1 = pages_accent_short[ 1 ];
        param_name2 = pages_accent_short[ 2 ];
        param_name3 = pages_accent_short[ 3 ];
      }else{
        param_name0 = pages_short[ act_menuNum*4    ];
        param_name1 = pages_short[ act_menuNum*4 +1 ];
        param_name2 = pages_short[ act_menuNum*4 +2 ];
        param_name3 = pages_short[ act_menuNum*4 +3 ];
      }       
      
    }else{
      // mute / unmute Instrument
      if( step_number == 12 ){
        Serial.print( "  mute:");
        Serial.println( act_instr );
        is_muted[act_instr] = ! is_muted[act_instr];  
        return;
      } 
      
      if( step_number == 13 ){
        change_pattern();
        return;
      } 

      if( step_number == 14 ){
        // Random Pattern
        random_pattern();
        return;
      } 
      if( step_number == 15 ){
        //  Additional Sound-Params
        Serial.print( "   Additional Sound-Params: ");
        Serial.println( act_instr );
        changeMenu( 1 );
        return;
      } 
         
    } 
  }

  // Sound-Menu
  if( act_menuNum == 1 ){
    if( step_number < 12 ){
      // select a Instrument 
      sequencer_new_instr( step_number  ); // this is a function on Core1 !?
    }else{
      // mute / unmute Instrument
      if( step_number == 12 ){
        Serial.print( "  mute:");
        Serial.println( act_instr );
        is_muted[act_instr] = ! is_muted[act_instr];  
        return;
      } 
      if( step_number == 13 ){
        change_pattern();
        return;
      } 
      if( step_number == 14 ){
        // Random Pattern
        random_pattern();
        return;
      } 
      if( step_number == 15 ){
        //  Additional Sound-Params
        Serial.print( "   Additional Sound-Params: ");
        Serial.println( act_instr );
        changeMenu( 0 );
        return;
      } 
         
    } 
  }
  
  // Global-Effects Menu
  if( act_menuNum == 2 ){
    // select a Instrumen 
  }
  // Velo-Menu
  if( act_menuNum == 3 ){
    // select a Instrumen 
  }  
  // Speed-Menu
  if( act_menuNum == 4 ){
    // select the Speed
  }
  // Bars-Menu
  if( act_menuNum == 5 ){
    // select a number of bars
  }
  // Notes-Menu
  if( act_menuNum == 6 ){
    // select a Midi-Notes to trigger external or get triggert internally 
  }
  // Scale-Menu
  if( act_menuNum == 7 ){
    // select a Scale 
  }
  // Sync-Menu
  if( act_menuNum == 8 ){
    // select a Sync-Modes 
  }  

  // Alternativ Toggle On/Off von  8 Sachen ...

}


void func2_but( uint8_t step_number ){
 // 
 Serial.print("Func2:"); 
 Serial.println( step_number ); 

  if( step_number <8 ){
    changeMenu( step_number );  
  }

  if( step_number == 15 ){
    Serial.print( "  next Menu:");
    Serial.println( act_instr );
    act_menuNum = act_menuNum+1; 
    changeMenu(); 
    return;
  } 
  if( step_number == 14 ){
    Serial.print( "  prevMenu:");
    Serial.println( act_instr );
    if( act_menuNum > 0 ){
      act_menuNum = act_menuNum-1;
    }else{
      act_menuNum = act_menuNum_max;
    } 
    changeMenu(); 
    return;
  } 
}

// active_track_num
// active_track_name

void changeMenu(){
  if( act_menuNum > act_menuNum_max ){
    act_menuNum = 0;
  } 
  changeMenu( act_menuNum );
  // act_menu = menus[ act_menuNum ];
  //Serial.println( "  changeMenu()");
}



void changeMenu( uint8_t new_menu_num ){
  // Function is coupled to Core 0


  val1_synced = false;
  val2_synced = false;
  val3_synced = false;
  val0_synced = false;

  act_menuNum = new_menu_num;
  if( act_menuNum > act_menuNum_max ){
    act_menuNum = act_menuNum_max;
  } 

  // Instrument-Menu
  if( act_menuNum ==0 ){
    step_pattern_1 = inotes1[ act_instr ];
    step_pattern_2 = inotes2[ act_instr ];
    
    // Sync Pot-Values
    if( act_instr > 0 ){
      // Volume
      param_val0 = volume_midi[ act_instr ];  
      // Decay
      param_val1 = decay_midi[ act_instr ];  
      // Pitch 
      param_val2 = pitch_midi[ act_instr ];  
      // Panorama
      param_val3 = pan_midi[ act_instr ];  
    }else{
      // Accent
      // Accent-Volume
      param_val0 = volume_midi[ act_instr ];  
      // Normal Instrument Volume-Factor
      param_val1 = decay_midi[ act_instr ];  
    }
    
    pcf_value1 = step_pattern_1;
    pcf_value2 = step_pattern_2;
    active_track_num = act_instr;
    active_track_name = shortInstr[ act_instr ];
    do_display_update = true;
  }

  // Sound-Menu Additional Sound/Instrument-Parameters
  if( act_menuNum ==1 ){    
    step_pattern_1 = inotes1[ act_instr ];
    step_pattern_2 = inotes2[ act_instr ];
    
    // Sync Pot-Values
    // MIDI-Note-Number
    param_val0 = midinote_in_out[ act_instr ];  
    // MIDI-Channel 1-16
    param_val1 = midichannel_in_out[ act_instr ];  
    // Attack-Offset
    param_val2 = attack_midi[ act_instr ];  
    // PitchDecay MOD
    param_val3 = pitchdecay_midi[ act_instr ];  
    
    pcf_value1 = step_pattern_1;
    pcf_value2 = step_pattern_2;
    active_track_num = act_instr;
    active_track_name = shortInstr[ act_instr ];
    do_display_update = true;
  }
  

  // Global Effects-Menu
  if( act_menuNum ==2 ){    
    // Filter-Freq
    patch_val0 = global_biCutoff_midi;
    // Filter-Reso
    patch_val1 = global_biReso_midi;
    // Bitcrush
    patch_val3 = global_bitcrush_midi;
    // Global Samplespeed
    patch_val3 = global_playbackspeed_midi;    
  }

  // Volume-Menu Volume for Accent-Steps and Normal Steps as a Factor, 
  if( act_menuNum ==3 ){    
    // "Acc", "Nor", "-","Set"
    // Accent-Volume
    // patch_val0 = bpm_pot_midi;
    // patch_val1 = bpm_pot_fine_midi;
    // patch_val2 = count_bars_midi;
    patch_val3 = program_midi;
    
  }

  // Speed-Menu
  if( act_menuNum ==4 ){    
    // Main-Speed
    patch_val0 = bpm_pot_midi;
    // Fine Speed Adjust
    patch_val1 = bpm_pot_fine_midi;   
    patch_val2 = count_bars_midi;
    patch_val3 = swing_midi;
    
  }
  /*
  param_name0 = pages[ act_menuNum*4    ];
  param_name1 = pages[ act_menuNum*4 +1 ];
  param_name2 = pages[ act_menuNum*4 +2 ];
  param_name3 = pages[ act_menuNum*4 +3 ];
  */

  if( act_menuNum==0 && act_instr==0){
    param_name0 = pages_accent_short[ 0 ];
    param_name1 = pages_accent_short[ 1 ];
    param_name2 = pages_accent_short[ 2 ];
    param_name3 = pages_accent_short[ 3 ];
  }else{
    param_name0 = pages_short[ act_menuNum*4    ];
    param_name1 = pages_short[ act_menuNum*4 +1 ];
    param_name2 = pages_short[ act_menuNum*4 +2 ];
    param_name3 = pages_short[ act_menuNum*4 +3 ];
  }
  act_menu = menus[ act_menuNum ];
  Serial.println( "  changeMenu()");

  sync_compared_values();
  
}
