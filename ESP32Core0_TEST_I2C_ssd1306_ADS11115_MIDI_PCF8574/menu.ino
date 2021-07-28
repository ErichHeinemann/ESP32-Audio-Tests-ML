// Menufunction
// Changes im Menu finden im Kontext des aktuellen Menus statt

// History:
// 2021-07-05 E.Heinemann Added this Menu-File to implement the structure for the menu, step by step.
//                        The Button underneeth the Rotary Encoder acts as a FUNCTION-Button to select something together with one of the 16 STEP-Buttons. On a Akai MPC1000/2500 this button is called Mode-Button



// 
void func1_but( uint8_t step_number ){
  Serial.print( "  act menu: ");
  Serial.print( act_menuNum);
  Serial.print( "  func_but:");
  Serial.println( step_number );

  // Instrument-Menu
  if( act_menuNum == 0 ){
    
    if( step_number < 12 ){
      // select a Instrumen 
      sequencer_new_instr( step_number  );
    }else{
      
      // mute / unmute Instrument
      if( step_number == 12 ){
        Serial.print( "  mute:");
        Serial.println( act_instr );
        is_muted[act_instr] = ! is_muted[act_instr];  
        return;
      } 
         
    } 
  }

  // Sound-Menu
  if( act_menuNum == 1 ){
    
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
  changeMenu(act_menuNum);
  // act_menu = menus[ act_menuNum ];
  //Serial.println( "  changeMenu()");
}

void changeMenu( uint8_t new_menu_num ){
  Serial.println( "  changeMenu( uint8_t new_menu_num )");
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
    // patch_val0 = bpm_pot_midi;
    // patch_val1 = bpm_pot_fine_midi;
  }

  // Speed-Menu
  if(  act_menuNum ==4 ){    
    // Main-Speed
    patch_val0 = bpm_pot_midi;
    // FIne_Speed
    patch_val1 = bpm_pot_fine_midi;
  }
  
  param_name0 = pages[ act_menuNum*4    ];
  param_name1 = pages[ act_menuNum*4 +1 ];
  param_name2 = pages[ act_menuNum*4 +2 ];
  param_name3 = pages[ act_menuNum*4 +3 ];
  
  act_menu = menus[ act_menuNum ];
  Serial.println( "  changeMenu()");
  
}


