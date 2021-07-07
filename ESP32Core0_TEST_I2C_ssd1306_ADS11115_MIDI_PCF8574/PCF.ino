// 3 PCF8574-ICs are used
// PCF 1 is used for the upper Row, Step 1-8 => Bit 0 - 7, it manages the LEDs and the Buttons!!
// PCF 2 is used for the lower Row, Step 9-16 => Bit 0 - 7, it manages the LEDs and the Buttons!!
// PCF 3 is used for the 5 Buttons and the Rotary Encoder, it is only used as Input!


// Variables for the Encoder:
boolean enc_pin_a_touched = false;
boolean enc_pin_b_touched = false;
boolean enc_pin_pb_touched = false;

// #ifndef _BV
// #define _BV(bit) (1 << (bit)) 
// #endif

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint8_t  lasttouched3[] = { 0,0,0,0 ,0,0,0,0 };
uint8_t  currtouched3[] = {  0,0,0,0 ,0,0,0,0 };

// Pin-Number used on the PCF3! 
uint8_t enc_pin_a  = 6; // center to ground
uint8_t enc_pin_b  = 5;  // center to ground
uint8_t enc_pin_pb = 7; // pushbutton

void PCF_setup(){  
  
  if (!PCF1.begin()){
    Serial.println("could not initialize PCF1...");
  }
  if (!PCF1.isConnected()){
    Serial.println(" PCF1 => not connected");
    while(1);
  }

  if (!PCF2.begin()){
    Serial.println("could not initialize PCF2...");
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
        if( ( pcf_value1 & (1 << i))==0 ){
          if( func_but_pressed == false ){
            bitSet( pcf_value1, i );
          }else{
            func_but( i );
          }
        }else{
          if( func_but_pressed == false ){
            bitClear( pcf_value1, i );
          }else{
            func_but( i );
          }
        }
      }
    }
  }
  
  byte tmp_pcf_value2 = PCF2.read8();
  if( tmp_pcf_value2 !=255  &&  tmp_pcf_value2_1 != tmp_pcf_value2 ){   
    for( int i=0; i <= 7; i++ ){
      if( (tmp_pcf_value2 & (1 << i))==0 ){
        if( ( pcf_value2 & (1 << i))==0 ){
          if( func_but_pressed == false ){
            bitSet( pcf_value2, i );
          }else{
            func_but( i +8);
          }
        }else{
          if( func_but_pressed == false ){
            bitClear( pcf_value2, i );
          }else{
            func_but( i +8 );
          }
        }
      }
    }
  }
 
  //pcf_value3 = tmp_pcf_value3;
  //Serial.println( pcf_value3 );
  
  if( do_display_update == false && ( pcf_value1 != pcf_value1_1 || pcf_value2 != pcf_value2_1 ) ){
    do_display_update = true;
  }

  //if( do_display_update == false && pcf_value3 != pcf_value3_1){
  //  do_display_update = true;
  //}
  
  pcf_value1_1 = pcf_value1;
  pcf_value2_1 = pcf_value2;

  
  tmp_pcf_value1_1 = tmp_pcf_value1;
  tmp_pcf_value2_1 = tmp_pcf_value2;


  // activate LEDs:
  PCF1.write8( pcf_value1 );
  PCF2.write8( pcf_value2 );
  
}



void readPCF3(){
  byte tmp_pcf_value3_but = tmp_pcf_value3_1; // PCF3.read8();
  if( tmp_pcf_value3_1_but != tmp_pcf_value3_but ){
    for( int i=0; i <= 7; i++ ){
      if( i!=enc_pin_a && i!=enc_pin_b ){
        
        lasttouched3[i] = currtouched3[i];
        if( ( tmp_pcf_value3_but & (1 << i))==0 ){
          currtouched3[i]=1;
          if( lasttouched3[i]==0 ){
            if( i == 0 ){
              func_but_pressed = true;
            }
            Serial.print("Button pressed ");
            Serial.println( i ); 
            
            // Mit dem Mechanismus wissen wir nicht, ob ein Button immer noch gedrückt ist. 
            // Wird für Menü-Fastenkombis und FUNC-Buttons benötigt!!   
          }
        }else{
          currtouched3[i]=0;
          if( lasttouched3[i]==1 ){
            if( i == 0 ){
              func_but_pressed = false;
            }
          } 
        }


        
      }
    }   
    tmp_pcf_value3_1_but = tmp_pcf_value3_but;
  }
}  




// faster Encoder
void readPCF_rotary_fast(){
  
  byte tmp_pcf_value3 = PCF3.read8();
  
  if(  tmp_pcf_value3_1 != tmp_pcf_value3 ){
    
    uint8_t i = enc_pin_a;
    lasttouched3[i] = currtouched3[i];

    if( ( tmp_pcf_value3 & (1 << i))==0 ){
      currtouched3[i]=1;
    }else{
      currtouched3[i]=0;
    }

    if( currtouched3[i]==1 && lasttouched3[i]==0 ){
      // Encoderepin A wurde bewegt
      if( enc_pin_a_touched==false ){
        enc_pin_a_touched = true;
        if( enc_pin_b_touched == true ){
           Serial.println("Enc turned right");  
           // Display und andere Abfragen verzögern, die Abfrage des Encoders hängt sonst
           ads_prescaler = 0;   
           display_prescaler = 0;
        }         
      }
    }      
    // if it *was* touched and now *isnt*, alert!
    if( currtouched3[i]==0 && lasttouched3[i]==1 ) {
      if( enc_pin_a_touched == true ){
        enc_pin_a_touched = false;       
      } 
    }
    
    // enc_pin_b
    i = enc_pin_b;
    lasttouched3[i] = currtouched3[i];
  
    if( ( tmp_pcf_value3 & (1 << i))==0 ){
      currtouched3[i]=1;
    }else{
      currtouched3[i]=0;
    }
  
    if( currtouched3[i]==1 && lasttouched3[i]==0 ){     
      // Encoderepin B wurde bewegt
      if( enc_pin_b_touched==false ){
        enc_pin_b_touched = true;
        if(  enc_pin_a_touched == true ){
           Serial.println("Enc turned left");    
           // Display und andere Abfragen verzögern, die Abfrage des Encoders hängt sonst
           ads_prescaler = 0;   
           display_prescaler = 0;
        }
      }         
    }
    
    // if it *was* touched and now *isnt*, alert!
    if( currtouched3[i]==0 && lasttouched3[i]==1 ) {
      if( enc_pin_b_touched == true ){
        enc_pin_b_touched = false;      
      } 
    }

    /*
    // Status der anderen Buttons
    i = enc_pin_pb;
    lasttouched3[i] = currtouched3[i];
    if( ( tmp_pcf_value3 & (1 << i))==0 ){
      currtouched3[i]=1;
    }else{
      currtouched3[i]=0;
    }    
    if( currtouched3[i]==1 && lasttouched3[i]==0 ){     
        Serial.println("Enc Button pressed");    
    }
    */
  }

  tmp_pcf_value3_1 = tmp_pcf_value3;
}  
