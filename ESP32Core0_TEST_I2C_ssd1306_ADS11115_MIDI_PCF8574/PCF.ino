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
uint8_t  lasttouched[] = { 0,0,0,0 ,0,0,0,0 };
uint8_t  currtouched[] = {  0,0,0,0 ,0,0,0,0 };

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
          bitSet( pcf_value1, i);
        }else{
          bitClear( pcf_value1, i);
        }
      }
    }
  }
  
  byte tmp_pcf_value2 = PCF2.read8();
  if( tmp_pcf_value2 !=255  &&  tmp_pcf_value2_1 != tmp_pcf_value2 ){   
    for( int i=0; i <= 7; i++ ){
      if( (tmp_pcf_value2 & (1 << i))==0 ){
        if( ( pcf_value2 & (1 << i))==0 ){
          bitSet( pcf_value2, i);
        }else{
          bitClear( pcf_value2, i);
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

void readPCF_rotary(){
  
 byte tmp_pcf_value3 = PCF3.read8();
  if(  tmp_pcf_value3_1 != tmp_pcf_value3 ){ 
    for( int i=0; i <= 7; i++ ){
      /*
      if( (tmp_pcf_value3 & (1 << i))==0 ){
        if( ( pcf_value3 & (1 << i))==0 ){
          bitSet( pcf_value3, i);
        }else{
          bitClear( pcf_value3, i);
        }
      }
      */
      lasttouched[i] = currtouched[i];

      if( ( tmp_pcf_value3 & (1 << i))==0 ){
        currtouched[i]=1;
      }else{
        currtouched[i]=0;
      }

      if( currtouched[i]==1 && lasttouched[i]==0 ){
        // irgendeine taste wurde gedrueckt
        if( i!=enc_pin_a && i!=enc_pin_b && i!=enc_pin_pb ){
          Serial.print(i); Serial.println(" touched");
        }  
        // Encoderepin A wurde bewegt
        if( i==enc_pin_a && enc_pin_a_touched==false ){
          enc_pin_a_touched = true;
          if( enc_pin_b_touched == true ){
             // Serial.println("Encoder turned right");  
             // Display und andere Abfragen verzögern, die Abfrage des Encoders hängt sonst
             ads_prescaler = 0;   
             display_prescaler = 0;
          }         
        }
        // Encoderepin B wurde bewegt
        if( i==enc_pin_b && enc_pin_b_touched==false ){
          enc_pin_b_touched = true;
          if(  enc_pin_a_touched == true ){
             // Serial.println("Encoder turned  left");    
             // Display und andere Abfragen verzögern, die Abfrage des Encoders hängt sonst
             ads_prescaler = 0;   
             display_prescaler = 0;
          }
        }
        // Encoderepin des Buttons wurde gedrueckt
        if( i == enc_pin_pb && enc_pin_pb_touched == false ){
          enc_pin_pb_touched = true;
          Serial.println("Encoder-button pressed");          
        }          
      }
      
      // if it *was* touched and now *isnt*, alert!
      if( currtouched[i]==0 && lasttouched[i]==1 ) {
        if( i!=enc_pin_a && i!=enc_pin_b && i!=enc_pin_pb ){
          Serial.print(i); Serial.println(" released");
        }
        if( i == enc_pin_a && enc_pin_a_touched == true ){
          enc_pin_a_touched = false;       
        } 
        if( i == enc_pin_b && enc_pin_b_touched == true ){
          enc_pin_b_touched = false;      
        } 
        if( i == enc_pin_pb && enc_pin_pb_touched == true ){
          enc_pin_pb_touched = false;   
          Serial.println("Encoder-button released");          
          
        } 
      }
      
    }
  }
  pcf_value3_1 = pcf_value3;
  tmp_pcf_value3_1 = tmp_pcf_value3;
}  
