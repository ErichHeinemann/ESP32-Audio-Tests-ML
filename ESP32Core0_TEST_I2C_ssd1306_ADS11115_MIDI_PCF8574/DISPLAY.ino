// The Display 128x64 shows:
// 4 values as Bars and Integer
// The Status of the 16 Steps
// The name of thee selected Menu - bottom right
// The current Status, recording, stop, play and tempo
// The number and name of the selected Instrument
// E.Heinemann 2021-06-04

// History
// 2021-06-04 E.Heinemann
// 2021-07-05 E.Heinemann replaced somee static test-Strings with real content


void update_display_bars(  ){
  
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);

  // display.setTextSize(2);             // Normal 1:1 pixel scale
  // If the values are not synced, then show both
  display.setCursor( 0, 0 );
  display.print(( param_name0 ));
  if( val0_synced != 1 ){
    display.print(" ");
    display.println( patch_val0 );   
    display.drawRect(0, 9, (int) param_val0/2, 3, SSD1306_INVERSE);
    display.drawRect(0, 12, (int) patch_val0/2, 3, SSD1306_INVERSE);
  }else{
    display.print(" ");
    display.println( param_val0 );   
    display.fillRect(0, 9, (int) param_val0/2, 6, SSD1306_INVERSE);
  }

  
  // Value 1
  if( param_name1 != no_display ){
    display.setCursor( 64, 0 );
    display.print( param_name1 );
    display.print(" ");

    if( act_menuNum != 1 ){ 
      if( val1_synced != 1 ){
        display.println( patch_val1 );   
        display.drawRect(64, 9, (int) param_val1/2, 3, SSD1306_INVERSE);
        display.drawRect(64, 12, (int) patch_val1/2, 3, SSD1306_INVERSE);
      }else{
        display.println( param_val1 );   
        display.fillRect(64, 9, (int) param_val1/2, 6, SSD1306_INVERSE);
      }
    }else{
      // In Menu 1 this is used to display the MIDI-Channel
      // MIDI-Channel has Values from 1 to 16
      if( val1_synced != 1 ){
        display.println( patch_val1 );   
        display.drawRect(64, 9, (int) (param_val1-1)/2, 3, SSD1306_INVERSE);
        display.drawRect(64, 12, (int) (patch_val1-1)/2, 3, SSD1306_INVERSE);
      }else{
        display.println( param_val1 );   
        display.fillRect(64, 9, (int) (patch_val1-1)/2, 6, SSD1306_INVERSE);
      }
    }
  }


  
  if( param_name2 != no_display ){
    display.setCursor( 0, 17 );
    display.print( param_name2 );
    display.print(" ");
    if( val2_synced != 1 ){
      display.println( patch_val2 );   
      display.drawRect(0, 26, (int) param_val2/2, 3, SSD1306_INVERSE);
      display.drawRect(0, 29, (int) patch_val2/2, 3, SSD1306_INVERSE);
    }else{
      display.println( param_val2 );   
      display.fillRect(0, 26, (int) param_val2/2, 6, SSD1306_INVERSE);
    }
  }
  
   if( param_name3 != no_display ){
    display.setCursor( 64, 17 ); 
    display.print( param_name3 );
    display.print(" ");
    if( val3_synced != 1 ){
      display.println( patch_val3 );   
      display.drawRect(64, 26, (int) param_val3/2, 3, SSD1306_INVERSE);
      display.drawRect(64, 29, (int) patch_val3/2, 3, SSD1306_INVERSE);
    }else{
      display.println( param_val3 );   
      display.fillRect(64, 26, (int) param_val3/2, 6, SSD1306_INVERSE);
    }
  }

  // Step-Positions
  uint8_t offset=35; 
  uint8_t step_width  =15;
  uint8_t step_height =5;
  // Upper 8 & lower 8 Steps
  int j = 0;


  if( is_muted[ act_instr ] == false ){
  display.setCursor( 64, 50 ); 
  display.println( step_pattern_1 );
    
    for (int i=0; i <= 7; i++){
      if( i==4 ) j = 3; // Insert a Offset after 4 Elements
      
      // is this bit set?
      if( step_pattern_1 & (1 << i) ){
        // draw unset step
        display.drawRect( i *step_width +j , offset, step_width-2, step_height, SSD1306_INVERSE);
      }else{
        // draw set step
        display.fillRect( i *step_width +j , offset, step_width-2, step_height, SSD1306_INVERSE);
      }
      
      if( step_pattern_2 & (1 << i) ){
        // draw unset step
        display.drawRect( i *step_width +j , offset+1+step_height, step_width-2, step_height, SSD1306_INVERSE);
      }else{
        // draw set step
        display.fillRect( i *step_width +j , offset+1+step_height, step_width-2, step_height, SSD1306_INVERSE);
      }
  
      /*
      // highlight the active step
      if( active_step < 8 && active_step == i ){
        display.drawLine( i *step_width +j +2, offset+2, i *step_width -5 +j +step_width, offset+2, SSD1306_INVERSE );   
      }
      if( active_step >= 8 && active_step-8 == i ){
        display.drawLine( i *step_width +j +2, offset+3+step_height, i *step_width -5 +j +step_width, offset+3+step_height, SSD1306_INVERSE );    
      }
      */

    }
  }else{
    display.setTextSize(2);
    display.setCursor( 5
    , offset-2 ); 
    display.println( "- muted -" );
    display.setTextSize(1);
  }

  // Current Menu
  display.setCursor( 100, 56 ); 

  display.println( act_menu );
  // display.println( "Inst");


  // Status 

    // and Speed
  if( act_menuNum == 4 ){
    display.setTextSize(1); 
    display.setCursor( 20, 50 ); 
    if( playBeats == true ){
      // Play
      display.print( ">");
    }else{  
      // Stopped
      display.print( "||"); 
    }    
    // Recording?
    // display.fillCircle( 69, 59, 4, SSD1306_INVERSE);
    display.setTextSize(2); 
    display.setCursor( 30, 50 ); 
    display.print( bpm );
    display.setTextSize(1);     
  }else{
    display.setCursor( 56, 57 ); 
    if( playBeats == true ){
      // Play
      display.print( ">");
    }else{  
      // Stopped
      display.print( "||");
    }
    // Recording?
    // display.fillCircle( 69, 59, 4, SSD1306_INVERSE);
    display.setTextSize(1); 
    display.setCursor( 69, 57 ); 
    display.print( bpm );
  }

  
  display.setCursor( 110, 56 ); 

  if( act_menuNum < 2 ){
    display.setCursor( 0, 57 ); 
    display.println( active_track_num );
    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setCursor( 15, 50 ); 
    display.println( active_track_name );
  }else{
   // normale Schrift  
    display.setCursor( 0, 57 ); 
    display.println( active_track_num  );
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setCursor( 0, 48 ); 
    display.println( active_track_name );
  }
    
  display.display();  
  // delay(100);
  digitalWrite( LED_PIN, HIGH ); // Low

}

/*
void update_display_fast(  ){
// Step-Positions
  uint8_t offset=35; 
  uint8_t step_width  =15;
  uint8_t step_height =5;
  // Upper 8 & lower 8 Steps
  int j = 0;
  for (int i=0; i <= 7; i++){
    if( i==4 ) j = 3; // Insert a Offset after 4 Elements
    
    // is this bit set?
    if( pcf_value1 & (1 << i) ){
      // draw unset step
      display.drawRect( i *step_width +j , offset, step_width-2, step_height, SSD1306_WHITE);
    }else{
      // draw set step
      display.fillRect( i *step_width +j , offset, step_width-2, step_height, SSD1306_WHITE);
    }
    
    if( pcf_value2 & (1 << i) ){
      // draw unset step
      display.drawRect( i *step_width +j , offset+1+step_height, step_width-2, step_height, SSD1306_WHITE);
    }else{
      // draw set step
      display.fillRect( i *step_width +j , offset+1+step_height, step_width-2, step_height, SSD1306_WHITE);
    }

    
    // highlight the active step
    if( active_step < 8 && active_step == i ){
      display.drawLine( i *step_width +j +2, offset+2, i *step_width -5 +j +step_width, offset+2, SSD1306_INVERSE );   
    }
    if( active_step >= 8 && active_step-8 == i ){
      display.drawLine( i *step_width +j +2, offset+3+step_height, i *step_width -5 +j +step_width, offset+3+step_height, SSD1306_INVERSE );    
    }
  }
  display.display();
    
}
*/
