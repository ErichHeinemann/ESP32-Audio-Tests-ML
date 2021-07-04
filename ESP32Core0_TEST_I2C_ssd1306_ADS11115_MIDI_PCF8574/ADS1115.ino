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
  adc0 = ( adc0*2 + ads.readADC_SingleEnded(0)  ) / 3;
  // delay(1);
  adc1 = ( adc1*2 + ads.readADC_SingleEnded(1)  ) / 3;
  // delay(1);
  adc2 = ( adc2*2 + ads.readADC_SingleEnded(2)  ) / 3;
  // delay(1);
  adc3 = ( adc3*2 + ads.readADC_SingleEnded(3)  ) / 3;

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
  if( do_display_update == false && ( adc0 > adc0_1 +adc_slope || adc0<adc0_1 -adc_slope   ) ){
    do_display_update = true;    
  }
  if( do_display_update == false && ( adc1 > adc1_1 +adc_slope || adc1<adc1_1 - adc_slope  ) ){
    do_display_update = true;    
  }
  if( do_display_update == false && ( adc2 > adc2_1 +adc_slope || adc2<adc2_1 -adc_slope   ) ){
    do_display_update = true;    
  }
  if( do_display_update == false && ( adc3 > adc3_1 +adc_slope || adc3<adc3_1 -adc_slope   ) ){
    do_display_update = true;    
  }

  if( do_display_update == true ){
    param_val0 = map( adc0, 0, 17600, 0, 127 );
    param_val1 = map( adc1, 0, 17600, 0, 127 );
    param_val2 = map( adc2, 0, 17600, 0, 127 );
    param_val3 = map( adc3, 0, 17600, 0, 127 );
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
