// The Display 128x64 shows:
// 4 values as Bars and Integer
// The Status of the 16 Steps
// The name of thee selected Menu - bottom right
// The current Status, recording, stop, play and tempo
// The number and name of the selected Instrument
// E.Heinemann 2021-06-04

void update_display_bars(  ){
  digitalWrite( LED_PIN, HIGH );
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
  display.setCursor( 64, 0 );
  display.print( param_name1 );
  display.print(" ");
  if( val1_synced != 1 ){
    display.println( patch_val1 );   
    display.drawRect(64, 9, (int) param_val1/2, 3, SSD1306_INVERSE);
    display.drawRect(64, 12, (int) patch_val1/2, 3, SSD1306_INVERSE);
  }else{
    display.println( param_val1 );   
    display.fillRect(64, 9, (int) param_val1/2, 6, SSD1306_INVERSE);
  }

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

  // Step-Positions
  uint8_t offset=36; 
  //display.setCursor( 110, offset ); 
  //display.println( pcf_value1 );
  //display.setCursor( 110, offset+8 ); 
  //display.println( pcf_value2 );
  uint8_t step_width  =15;
  uint8_t step_height =5;
  // Upper 8 & lower 8 Steps
  int j = 0;
  for (int i=0; i <= 7; i++){
    if( i==4 ) j = 3;
    if( pcf_value1 & (1 << i) ){
      // draw unset step
      display.drawRect( i *step_width +j , offset, step_width-2, step_height, SSD1306_INVERSE);
    }else{
      // draw set step
      display.fillRect( i *step_width +j , offset, step_width-2, step_height, SSD1306_INVERSE);
    }
    if( pcf_value2 & (1 << i) ){
      // draw unset step
      display.drawRect( i *step_width +j , offset+1+step_height, step_width-2, step_height, SSD1306_INVERSE);
    }else{
      // draw set step
      display.fillRect( i *step_width +j , offset+1+step_height, step_width-2, step_height, SSD1306_INVERSE);
    }
    
  }

  // Current Menu
  display.setCursor( 110, 56 ); 
  display.println( "Inst");

  // Status and Speed
  display.setCursor( 64, 56 ); 
  // Play
  // display.print( ">");
  // Stopped
  // display.print( "||");

  // Recording?
  display.fillCircle( 69, 59, 4, SSD1306_INVERSE);
  
  display.setCursor( 80, 56 ); 
  display.print( "131");
  
  
  display.setCursor( 110, 56 ); 
  //  display.println( pcf_value3 );

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setCursor( 0, 50 ); 
  display.println( active_track_num );
  display.setCursor( 12, 50 ); 
  display.println( active_track_name );
    
  display.display();  
  // delay(100);
  digitalWrite( LED_PIN, LOW );

}
