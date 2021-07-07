// Menufunction
// Changes im Menu finden im Kontext des aktuellen Menus statt

// History:
// 2021-07-05 E.Heinemann Added this Menu-File to implement the structure for the menu, step by step.
//                        The Button underneeth the Rotary Encoder acts as a FUNCTION-Button to select something together with one of the 16 STEP-Buttons. On a Akai MPC1000/2500 this button is called Mode-Button



// 
void func_but( uint8_t step_number ){
  Serial.print( step_number );
  Serial.println( "  func_but");
  // Instrument-Menu
  if( act_menuNum == 0 ){
    // select a Instrumen 
  }
  // Sound-Menu
  if( act_menuNum == 1 ){
    // select a Instrumen 
  }
  // Global-Menu
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

// active_track_num
// active_track_name

void changeMenu(){
  act_menu = menus[ act_menuNum ];
  Serial.println( "  changeMenu()");
}

void changeMenu( uint8_t new_menu_num ){
  Serial.println( "  changeMenu( uint8_t new_menu_num )");
}
