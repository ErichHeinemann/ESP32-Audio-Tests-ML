// History:
// 2021-07-05 E.Heinemann less access to I2C, third PCF8574 with Rotary-Encoder is checked in a high frequency - Rotary Encoder provides results via Serial Monitor for now

#define DEBUG_PCF

// 3 PCF8574-ICs are used
// PCF 1 is used for the upper Row, Step 1-8 => Bit 0 - 7, it manages the LEDs and the Buttons!!
// PCF 2 is used for the lower Row, Step 9-16 => Bit 0 - 7, it manages the LEDs and the Buttons!!
// PCF 3 is used for the 5 Buttons and the Rotary Encoder, it is only used as Input!



// Variables for the Encoder:
boolean enc_pin_a_touched = false;
boolean enc_pin_b_touched = false;
boolean enc_pin_pb_touched = false;

uint8_t last_direction = 1; // 0=left, 1=buggy, 2=right

// #ifndef _BV
// #define _BV(bit) (1 << (bit)) 
// #endif

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint8_t  lasttouched3[] = { 0,0,0,0 ,0,0,0,0 };
uint8_t  currtouched3[] = {  0,0,0,0 ,0,0,0,0 };

// Pin-Number used on the PCF3! 
uint8_t enc_pin_a  = 6; // center to ground
uint8_t enc_pin_b  = 5; // center to ground
uint8_t enc_pin_pb = 7; // pushbutton

void PCF_setup(){  
  
  if (!PCF1.begin()){
#ifdef DEBUG_PCF      
    Serial.println("could not initialize PCF1...");
#endif    
  }
  if (!PCF1.isConnected()){
#ifdef DEBUG_PCF       
    Serial.println(" PCF1 => not connected");
#endif    
    while(1);
  }

  if (!PCF2.begin()){
#ifdef DEBUG_PCF    
    Serial.println("could not initialize PCF2...");
#endif    
  }
  if (!PCF2.isConnected()){
    Serial.println(" PCF2 => not connected");
    while(1);
  }
  if (!PCF3.begin()){
    Serial.println("could not initialize PCF3...");
  }
  if (!PCF3.isConnected()){
    Serial.println(" PCF3 => not connected");
    while(1);
  }
  // Inverse Logic, 
  pcf_value1 = 255;
  pcf_value2 = 255;

}

void readPCF(){
  PCF1.write8(255);
  PCF2.write8(255);
  //PCF3.write8(255);
  
  byte tmp_pcf_value1 = PCF1.read8();
  if( tmp_pcf_value1 !=255  &&  tmp_pcf_value1_1 != tmp_pcf_value1 ){   
    for( int i=0; i <= 7; i++ ){
      if( (tmp_pcf_value1 & (1 << i))==0 ){
        if( ( step_pattern_1 & (1 << i))==0 ){
          if( func1_but_pressed == false && func2_but_pressed == false ){
            bitSet( step_pattern_1, i );
          }else{
            if( func1_but_pressed == true && func2_but_pressed == false ){
              func1_but( i );
            }else{
              func2_but( i );
            }
          }
        }else{
          if( func1_but_pressed == false && func2_but_pressed == false){
            bitClear( step_pattern_1, i );
          }else{
            if( func1_but_pressed == true && func2_but_pressed == false ){
              func1_but( i );
            }else{
              func2_but( i );
            }
          }
        }
      }
    }
    pcf_value1 = step_pattern_1;
  }
  
  byte tmp_pcf_value2 = PCF2.read8();
  if( tmp_pcf_value2 !=255  &&  tmp_pcf_value2_1 != tmp_pcf_value2 ){   
    for( int i=0; i <= 7; i++ ){
      if( (tmp_pcf_value2 & (1 << i))==0 ){
        if( ( step_pattern_2 & (1 << i))==0 ){
          if( func1_but_pressed == false && func2_but_pressed == false ){
            bitSet( step_pattern_2, i );
          }else{
            if( func1_but_pressed == true && func2_but_pressed == false ){
              func1_but( i +8);
            }else{
              func2_but( i +8);
            }
          }
        }else{
          if( func1_but_pressed == false && func2_but_pressed == false ){
            bitClear( step_pattern_2, i );
          }else{
            if( func1_but_pressed == true && func2_but_pressed == false ){
              func1_but( i +8);
            }else{
              func2_but( i +8);
            }
          }
        }
      }
    }
    
  }
 
  
  if( do_display_update == false && ( pcf_value1 != pcf_value1_1 || pcf_value2 != pcf_value2_1 ) ){
    do_display_update = true;
  }

  pcf_value1 = step_pattern_1;
  pcf_value2 = step_pattern_2;
  pcf_value1_1 = pcf_value1;
  pcf_value2_1 = pcf_value2;

  // Values auf die Bytes des aktuellen Instruments übertragen!!
  // From UI-Core to Sequencer-Core

  sequencer_update_track( step_pattern_1, step_pattern_2 );
  
  tmp_pcf_value1_1 = tmp_pcf_value1;
  tmp_pcf_value2_1 = tmp_pcf_value2;

  // activate LEDs, but if the beat is playing, invert the current step
  if( playBeats == true ){
    pcf_update_leds();
  }else{
    PCF1.write8( pcf_value1 );
    PCF2.write8( pcf_value2 );
  }  
}

void pcf_update_leds(){
  
  pcf_value1 = step_pattern_1;
  pcf_value2 = step_pattern_2;
 
  // activate LEDs, but if the beat is playing, invert the current step
  if( playBeats == true ){

    if( active_step < 8 ){
      if( ( step_pattern_1 & (1 << active_step))==0 ){
        bitSet( pcf_value1, active_step );
      }else{
        bitClear( pcf_value1, active_step );
      }
    }else{
      if( ( step_pattern_2 & (1 << (active_step -8)))==0 ){
        bitSet( pcf_value2, (active_step-8) );
      }else{
        bitClear( pcf_value2, (active_step-8) );
      }
    }
  }  
  PCF1.write8( pcf_value1 );
  PCF2.write8( pcf_value2 );  

}

void readPCF3(){
  byte tmp_pcf_value3_but = tmp_pcf_value3_1; // PCF3.read8(); - No newe Read neeeeded, the last value is in tmp_pcf_value3_1 which is faster
  if( tmp_pcf_value3_1_but != tmp_pcf_value3_but ){
    for( int i=0; i <= 7; i++ ){
      if( i!=enc_pin_a && i!=enc_pin_b ){
        
        lasttouched3[i] = currtouched3[i];
        if( ( tmp_pcf_value3_but & (1 << i))==0 ){
          currtouched3[i]=1;
          if( lasttouched3[i]==0 ){
            if( i == 0 ){
              func1_but_pressed = true;
              if( act_menuNum > 1 ){ 
                changeMenu( 0 );
              }
            }
            if( i == 1 ){
              func2_but_pressed = true;
            }            
#ifdef DEBUG_PCF              
            Serial.print("But press ");
            Serial.println( i ); 
#endif            
            if( i==2 ){
              // Play?
              if( playBeats == false ){
                sequencer_start();  
                external_clock_in = false;
              }else{
                sequencer_stop();
                external_clock_in = false;
                // sequencer_start();                
              }
            } 

            if( i==3 ){
              // Continue?
              if( playBeats == false ){
                sequencer_continue();
                external_clock_in = false;
              }else{
                sequencer_stop();
                external_clock_in = false;
              }
            }              
            
            // Mit dem Mechanismus wissen wir nicht, ob ein Button immer noch gedrückt ist. 
            // Wird für Menü-Fastenkombis und FUNC-Buttons benötigt!!   
          }
        }else{
          currtouched3[i]=0;
          if( lasttouched3[i]==1 ){
            if( i == 0 ){
              func1_but_pressed = false;
            }
            if( i == 1 ){
              func2_but_pressed = false;
            }
            
          } 
        }

      }
    }   
    tmp_pcf_value3_1_but = tmp_pcf_value3_but;
  }
}  




// faster Encoder
int16_t readPCF_rotary_fast( uint16_t old_value ){
  
  tmp_pcf_value3 = PCF3.read8();
  
  if(  tmp_pcf_value3_1 != tmp_pcf_value3 ){
    
    //uint8_t i = enc_pin_a;
    lasttouched3[enc_pin_a] = currtouched3[enc_pin_a];

    if( ( tmp_pcf_value3 & (1 << enc_pin_a))==0 ){
      currtouched3[enc_pin_a]=1;
    }else{
      currtouched3[enc_pin_a]=0;
    }

    if( currtouched3[enc_pin_a]==1 && lasttouched3[enc_pin_a]==0 ){
      // Encoderepin A was moved
      if( enc_pin_a_touched==false ){
        enc_pin_a_touched = true;
        if( enc_pin_b_touched == true ){
          if( last_direction != 0 ){
            old_value = old_value +1;
#ifdef DEBUG_PCF         
            Serial.println("Enc right");
#endif
            last_direction = 2; // right
          }else{
            last_direction = 1; // neutral
          }
          // Display und andere Abfragen verzögern, die Abfrage des Encoders hängt sonst
          ads_prescaler = 0;   
          display_prescaler = 0;
        }         
      }
      tmp_pcf_value3_1 = tmp_pcf_value3;
      return old_value;
    }      
    // if it *was* touched and now *isnt*, alert!
    if( currtouched3[enc_pin_a]==0 && lasttouched3[enc_pin_a]==1 ) {
      if( enc_pin_a_touched == true ){
        enc_pin_a_touched = false;       
      }
      //tmp_pcf_value3_1 = tmp_pcf_value3;
      //return old_value;
    }
    
    // now, the same for enc_pin_b
    //i = enc_pin_b;
    lasttouched3[enc_pin_b] = currtouched3[enc_pin_b];
  
    if( ( tmp_pcf_value3 & (1 << enc_pin_b))==0 ){
      currtouched3[enc_pin_b]=1;
    }else{
      currtouched3[enc_pin_b]=0;
    }
  
    if( currtouched3[enc_pin_b]==1 && lasttouched3[enc_pin_b]==0 ){     
      // Encoderepin B wurde bewegt
      if( enc_pin_b_touched==false ){
        enc_pin_b_touched = true;
        if(  enc_pin_a_touched == true ){
          if( last_direction != 2 ){
            old_value = old_value -1;
            last_direction = 0; // left
#ifdef DEBUG_PCF           
             Serial.println("Enc left");  
#endif             
          }else{
            last_direction = 1;    // neutral        
          }
           // Display und andere Abfragen verzögern, die Abfrage des Encoders hängt sonst
           ads_prescaler = 0;   
           display_prescaler = 0;
        }
      }
      tmp_pcf_value3_1 = tmp_pcf_value3;
      return old_value;  
    }
    
    // if it *was* touched and now *isnt*, alert!
    if( currtouched3[enc_pin_b]==0 && lasttouched3[enc_pin_b]==1 ) {
      if( enc_pin_b_touched == true ){
        enc_pin_b_touched = false;      
      } 
    }
    tmp_pcf_value3_1 = tmp_pcf_value3;
    return old_value;
  }

  tmp_pcf_value3_1 = tmp_pcf_value3;

  return old_value;
}  
