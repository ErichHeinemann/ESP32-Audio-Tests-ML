/*
 * this file includes the implementation of the sample player
 * all samples are loaded from littleFS stored on the external flash
 *
 * Author: Marcel Licence
 * 
 * Modifications:
 * 2021-04-05 E.Heinemann changed BLOCKSIZE from 2024 to 1024
 *             , added DEBUG_SAMPLER
 *             , added sampleRate to the structure of samplePlayerS to optimize the pitch based on lower samplerates
 * 2021-07-28 E.Heinemann, added pitch-decay and pan
 * 2021-08-03 E.Heinemann, changed Accent/normal Velocity in the code
 */

#include <Arduino.h>
#include "FS.h"
#include <LITTLEFS.h>

#define CONFIG_LITTLEFS_CACHE_SIZE 512

/* use define to dump midi data */
#define DEBUG_SAMPLER

// If Blocksize us set to 2048, the limit of SAMPLECNT is 12
#define BLOCKSIZE  (2048*1) /* only multiples of 2, otherwise the rest will not work */
#define SAMPLECNT 12

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

/* You only need to format LITTLEFS the first time you run a
   test or else use the LITTLEFS plugin to create a partition
   https://github.com/lorol/arduino-esp32littlefs-plugin */

#define FORMAT_LITTLEFS_IF_FAILED true

struct samplePlayerS{
    char filename[32];
    File file;
    uint32_t sampleRate;
    uint8_t preloadData[ BLOCKSIZE ];

    uint32_t sampleSize;
    float samplePosF;
    uint32_t samplePos;
    uint32_t lastDataOut;
    uint8_t data[ 2 * BLOCKSIZE ];
    bool active;
    uint32_t sampleSeek;
    uint32_t dataIn;
    float volume; // Volume of Track
    float signal;

    float decay;
    float vel;  // temp Velocity of NoteOn?
    float pitch;
    // float release;
    float pan;
    // float volume_float; // Volume of Track 0.0 - ?
    
    uint8_t decay_midi;
    uint8_t volume_midi; // Volume of Track
    uint8_t pitch_midi;
    uint8_t attack_midi; // offset for samples
    uint8_t pan_midi;
    boolean is_muted;

    float pitchdecay = 0.0f;
    uint8_t pitchdecay_midi;
  
};

struct samplePlayerS samplePlayer[ SAMPLECNT ];

/*
 * ###############################################################################################
 * #
 * # Configure these Parameters!! progNumber & countPrograms
 * #
 * ###############################################################################################
 */

uint8_t progNumber = 0; // first subdirectory in /data 
uint8_t countPrograms = 5;

// float global_pitch_decay = 0.0f; // good from -0.2 to +1.0

uint32_t sampleInfoCount = 0; /*!< storing the count if found samples in file system */
float slowRelease; /*!< slow releasing signal will be used when sample playback stopped */


void Sampler_ScanContents(fs::FS &fs, const char *dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if( !root ){
        Serial.println("- failed to open directory");
        return;
    }
    if( !root.isDirectory() ){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while( file ){
        if( file.isDirectory() ){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if( levels ){
                Sampler_ScanContents(fs, file.name(), levels - 1);
            }
        }else{
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());

            if( sampleInfoCount < SAMPLECNT ){
                strncpy( samplePlayer[ sampleInfoCount ].filename, file.name(), 32);
                sampleInfoCount ++;
                String fname = file.name();                
                shortInstr[ sampleInfoCount ] = fname.substring(7, 10);
            }
        }
        delay(1);
        file = root.openNextFile();
    }
}


/*
 * union is very handy for easy conversion of bytes to the wav header information
 */
union wavHeader{
    struct{
        char riff[4];
        uint32_t fileSize; /* 22088 */
        char waveType[4];
        char format[4];
        uint32_t lengthOfData;
        uint16_t numberOfChannels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t bytesPerSample;
        uint16_t bitsPerSample;
        char dataStr[4];
        uint32_t dataSize; /* 22052 */
    };
    uint8_t wavHdr[44];
};


inline void Sampler_Init(){
    if( !LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LITTLEFS Mount Failed");
        return;
    }
    String myDir = "/" + (String)progNumber + "/";
    // myDir.concat(  (String)progNumber ).concat("/");

    sampleInfoCount = 0;
    Sampler_ScanContents(LITTLEFS, myDir.c_str() , 5);


    Serial.println("---\nListSamples:");

    for (int i = 0; i < sampleInfoCount; i++ ){
        Serial.printf( "s[%d]: %s\n", i, samplePlayer[i].filename );

        delay(10);

        File f = LITTLEFS.open( samplePlayer[i].filename );

        if( f ){
            union wavHeader wav;
            int j = 0;
            while( f.available() && ( j < sizeof(wav.wavHdr)) ){
                wav.wavHdr[j] = f.read();
                j++;
            }

            j = 0;
            /* load first block of sample data */
            while( f.available() && ( j < BLOCKSIZE) ){
                samplePlayer[i].preloadData[j] = f.read();
                j++;
            }

            samplePlayer[i].file = f; /* store file pointer for future use */
            samplePlayer[i].sampleRate = wav.sampleRate;
#ifdef DEBUG_SAMPLER
            Serial.printf("fileSize: %d\n",      wav.fileSize);
            Serial.printf("lengthOfData: %d\n",  wav.lengthOfData);
            Serial.printf("numberOfChannels: %d\n", wav.numberOfChannels);
            Serial.printf("sampleRate: %d\n",    wav.sampleRate);
            Serial.printf("byteRate: %d\n",      wav.byteRate);
            Serial.printf("bytesPerSample: %d\n", wav.bytesPerSample);
            Serial.printf("bitsPerSample: %d\n", wav.bitsPerSample);
            Serial.printf("dataSize: %d\n",      wav.dataSize);
            Serial.printf("dataInBlock: %d\n",   j);
#endif            
            samplePlayer[i].sampleSize =         wav.dataSize; /* without mark section and size info */
            samplePlayer[i].sampleSeek = 0xFFFFFFFF;
        }else{
            Serial.printf("error openening file!\n");
        }
    }

    for( int i = 0; i < SAMPLECNT; i++ ){
        samplePlayer[i].sampleSeek = 0xFFFFFFFF;
        samplePlayer[i].active = false;

        decay_midi[i+1] = 127;
        samplePlayer[i].decay_midi = 64;
        samplePlayer[i].decay = 1.0f;

        attack_midi[i+1] = 0;
        samplePlayer[i].attack_midi = 0;
        
        volume_midi[i+1] = 64;
        samplePlayer[i].volume_midi = 127;

        pan_midi[i+1] = 64;
        samplePlayer[i].pan_midi = 64;
        samplePlayer[i].pan = 0.5;

        pitch_midi[i+1] = 64;
        samplePlayer[i].pitch_midi = 64;
        if( samplePlayer[i].sampleRate > 0 ){
          samplePlayer[i].pitch = 1.0f / SAMPLE_RATE * samplePlayer[i].sampleRate;
        } 
    };
}


uint8_t selectedNote = 0;


inline void Sampler_SelectNote( uint8_t note ){
  selectedNote = note % sampleInfoCount;
}


inline void Sampler_SetPan_Midi( uint8_t data1){
  samplePlayer[ selectedNote ].pan_midi = data1;
  float value = NORM127MUL * (float)data1;
  samplePlayer[ selectedNote ].pan =  value;
#ifdef DEBUG_SAMPLER
  Serial.printf("Sampler - Note[%d].pan: %0.2f\n",  selectedNote, samplePlayer[ selectedNote ].pan );
#endif  
}


inline void Sampler_SetDecay_Midi( uint8_t data1){
  samplePlayer[ selectedNote ].decay_midi = data1;
  float value = NORM127MUL * (float)data1;
  samplePlayer[ selectedNote ].decay = 1 - (0.000005 * pow( 5000, 1.0f - value) );
#ifdef DEBUG_SAMPLER
  Serial.printf("Sampler - Note[%d].decay: %0.2f\n",  selectedNote, samplePlayer[ selectedNote ].decay);
#endif  
}

inline void Sampler_SetVolume_Midi( uint8_t data1){
  samplePlayer[ selectedNote ].volume_midi = data1;
}

uint16_t Sampler_GetSoundSamplerate(){
  return samplePlayer[ selectedNote ].sampleRate;
}

uint8_t Sampler_GetSoundDecay_Midi(){
  return samplePlayer[ selectedNote ].decay_midi;
}

uint16_t Sampler_GetSoundPan_Midi(){
  return samplePlayer[ selectedNote ].pan_midi;
}

uint8_t Sampler_GetSoundPitch_Midi(){
  return samplePlayer[ selectedNote ].pitch_midi;
}

uint8_t Sampler_GetSoundVolume_Midi(){
  return samplePlayer[ selectedNote ].volume_midi;
}

void Sampler_SetSoundPitch_Midi( uint8_t value){
  Serial.println("Pitch");
  samplePlayer[ selectedNote ].pitch_midi = value;
  Sampler_SetSoundPitch( NORM127MUL * value );
}

void Sampler_SetSoundPitch(float value){
  samplePlayer[ selectedNote ].pitch = pow( 2.0f, 4.0f * ( value - 0.5f ) );
#ifdef DEBUG_SAMPLER  
  Serial.printf("Sampler - Note[%d] pitch: %0.3f\n",  selectedNote, samplePlayer[ selectedNote ].pitch );
#endif 
}

// Offset for the Sample-Playback to cut the sample from the left
void Sampler_SetAttack_Midi( uint8_t value ){
  samplePlayer[selectedNote].attack_midi = value;
}


inline void Sampler_NoteOn( uint8_t note, uint8_t vol ){
  
    /* check for null to avoid division by zero */
    if( sampleInfoCount == 0 ){
        return;
    }
    int j = note % sampleInfoCount;

    if( is_muted[ j+1 ]== true){
      return;
    }

#ifdef DEBUG_SAMPLER
    Serial.printf("note %d on volume %d\n", note, vol );
    Serial.printf("Filename: %s \n", samplePlayer[ j ].filename );    
#endif
    /*
    if( global_pitch_decay_midi != global_pitch_decay_midi_old ){
      global_pitch_decay_midi_old = global_pitch_decay_midi;
      if( global_pitch_decay_midi < 63 ){ 
        global_pitch_decay = (float) (65-global_pitch_decay_midi)/100; // good from -0.2 to +1.0
      }else if( global_pitch_decay_midi > 65 ){ 
        global_pitch_decay = (float) global_pitch_decay_midi/65; // good from -0.2 to +1.0
      }else{
        global_pitch_decay = 0.0f;  
      }
    }
    */

    if( volume_midi[ j+1 ] != samplePlayer[ j ].volume_midi ){
#ifdef DEBUG_SAMPLER
      Serial.print("Volume");
      Serial.println( j );      
      Serial.print(" samplePlayer");
      Serial.println( volume_midi[ j+1 ] );
#endif   
      samplePlayer[ j ].volume_midi = volume_midi[ j+1 ];
    }

    if( decay_midi[ j+1 ] != samplePlayer[ j ].decay_midi ){
#ifdef DEBUG_SAMPLER
      Serial.print("Decay");
      Serial.println( j );      
      Serial.print(" samplePlayer");
      Serial.println( decay_midi[ j+1 ] );
#endif     
      samplePlayer[ j ].decay_midi = decay_midi[ j+1 ];
      float value = NORM127MUL * decay_midi[ j+1 ];
      samplePlayer[ j ].decay = 1 - (0.000005 * pow( 5000, 1.0f - value) );
    }

    if( pitch_midi[ j+1 ] != samplePlayer[ j ].pitch_midi ){
#ifdef DEBUG_SAMPLER
      Serial.print("Pitch");
      Serial.println( j );      
      Serial.print(" samplePlayer");
      Serial.println( pitch_midi[ j+1 ] );
#endif   
      samplePlayer[ j ].pitch_midi = pitch_midi[ j+1 ];
      float value = NORM127MUL * pitch_midi[ j+1 ];
      samplePlayer[ j ].pitch = pow( 2.0f, 4.0f * ( value - 0.5f ) );
    }

    if( pan_midi[ j+1 ] != samplePlayer[ j ].pan_midi ){
#ifdef DEBUG_SAMPLER
      Serial.print("Pan");
      Serial.println( j );      
      Serial.print(" samplePlayer");
      Serial.println( pan_midi[ j+1 ] );
#endif   
      samplePlayer[ j ].pan_midi = pan_midi[ j+1 ];
      float value = NORM127MUL * pan_midi[ j+1 ];
      samplePlayer[ j ].pan = value;
    }

    if( attack_midi[ j+1 ] != samplePlayer[ j ].attack_midi ){
#ifdef DEBUG_SAMPLER
      Serial.print("Attack Offset");
      Serial.println( j );      
      Serial.print(" samplePlayer");
      Serial.println( attack_midi[ j+1 ] );
#endif   
      samplePlayer[ j ].attack_midi = attack_midi[ j+1 ];
    }
    
    if( pitchdecay_midi[ j+1 ] != samplePlayer[ j ].pitchdecay_midi ){
 
      samplePlayer[ j ].pitchdecay_midi = pitchdecay_midi[ j+1 ];
      samplePlayer[ j ].pitchdecay = 0.0f; // default
      if( samplePlayer[ j ].pitchdecay_midi < 63 ){ 
        samplePlayer[ j ].pitchdecay = (float) (63 - samplePlayer[ j ].pitchdecay_midi ) /20.0f; // good from -0.2 to +1.0
      }else if( samplePlayer[ j ].pitchdecay_midi > 65 ){ 
        samplePlayer[ j ].pitchdecay = (float) -( samplePlayer[ j ].pitchdecay_midi -65) /30.0f; // good from -0.2 to +1.0
      }
#ifdef DEBUG_SAMPLER
      Serial.print("PitchDecay");
      Serial.println( j );      
      Serial.print(" samplePlayer ");
      Serial.print( pitchdecay_midi[ j+1 ] );
      Serial.print(" FloatValue: " );
      Serial.println( samplePlayer[ j ].pitchdecay );
#endif  
    }


    struct samplePlayerS *newSamplePlayer = &samplePlayer[j];
    
    if( newSamplePlayer->active ){
        /* add last output signal to slow release to avoid noise */
        slowRelease = newSamplePlayer->signal;
    }

    newSamplePlayer->samplePosF = 0.0f;
    newSamplePlayer->samplePos  = 0;
    newSamplePlayer->lastDataOut = BLOCKSIZE; /* trigger loading second half */
    
    newSamplePlayer->volume = vol * NORM127MUL * newSamplePlayer->volume_midi * NORM127MUL;
    newSamplePlayer->vel    = 1.0f;
    newSamplePlayer->dataIn = 0;
    newSamplePlayer->sampleSeek = 44 + 2 *newSamplePlayer->attack_midi; // 16 Bit-Samples wee need a offset which fits the 2 bytes
 
    memcpy( newSamplePlayer->data, newSamplePlayer->preloadData, BLOCKSIZE );
    newSamplePlayer->active = true;
    // ML newSamplePlayer->file.seek( BLOCKSIZE + 44, SeekSet ); /* seek ack to beginning -> after pre load data */

    // Wir starten ggf. etwas weiter hinten im Sample um Attack-Phasen abzuschneinden.
    newSamplePlayer->file.seek( BLOCKSIZE + newSamplePlayer->sampleSeek , SeekSet ); /* seek ack to beginning -> after pre load data */

    
    // Das waere auch der Einstiegspunkt, wenn man Loopen wollte.
}

inline void Sampler_NoteOff( uint8_t note ){
    /*
     * nothing to do yet
     * we could stop samples if we want to
     */
       if( sampleInfoCount == 0 ){
        return;
    }
    int j = note % sampleInfoCount;
    // samplePlayer[j]->active = false;
}

float sampler_playback = 1.0f;

void Sampler_SetPlaybackSpeed_Midi( uint8_t value ){
  Sampler_SetSoundPitch( (float) NORM127MUL * value );
}

void Sampler_SetPlaybackSpeed( float value ){
    value = pow( 2.0f, 4.0f * (value - 0.5) );
    Serial.printf( "Sampler_SetPlaybackSpeed: %0.2f\n", value );
    sampler_playback = value;
}

void Sampler_SetProgram( uint8_t prog ){
  progNumber = prog % countPrograms;
  Sampler_Init();
}

inline void Sampler_Process( float *left, float *right ){
    float signal_l = 0.0f;
    signal_l += slowRelease;
    float signal_r = 0.0f;
    signal_r += slowRelease;
    
    slowRelease = slowRelease * 0.99; /* go slowly to zero */

    for( int i = 0; i < SAMPLECNT; i++ ){

        if( samplePlayer[i].active  ){
            samplePlayer[i].samplePos = samplePlayer[i].samplePosF;
            samplePlayer[i].samplePos -= samplePlayer[i].samplePos % 2;
            uint32_t dataOut = samplePlayer[i].samplePos & ((BLOCKSIZE * 2) - 1); /* going through all data and repeat */

            /* first byte of second half */
            if( (dataOut >= BLOCKSIZE) && ( samplePlayer[i].lastDataOut < BLOCKSIZE )){
                samplePlayer[i].file.read( &samplePlayer[i].data[0], BLOCKSIZE );
            }
            /* first byte of second half */
            if( (dataOut < BLOCKSIZE ) && ( samplePlayer[i].lastDataOut >= BLOCKSIZE ) ){
                samplePlayer[i].file.read(&samplePlayer[i].data[BLOCKSIZE], BLOCKSIZE);
            }
            samplePlayer[i].lastDataOut = dataOut;

            /*
             * reconstruct signal from data
             */
            union{
                uint16_t u16;
                int16_t s16;
            } sampleU;

            sampleU.u16 = (((uint16_t)samplePlayer[i].data[dataOut + 1]) << 8U) + (uint16_t)samplePlayer[i].data[dataOut + 0];
            samplePlayer[i].signal = samplePlayer[i].volume * ((float)sampleU.s16) * (1.0f / ((float)(0x9000)) );
            signal_l += samplePlayer[i].signal * samplePlayer[i].vel * ( 1- samplePlayer[i].pan );
            signal_r += samplePlayer[i].signal * samplePlayer[i].vel *  samplePlayer[i].pan;

            // Filter per SamplePlayer?
            // ToBeDone 

            samplePlayer[i].vel *= samplePlayer[i].decay;

            samplePlayer[i].samplePos += 2; /* we have consumed two bytes */

            // If a glolab_pitch_decay is used: samplePlayer[i].samplePosF += 2.0f * sampler_playback * ( samplePlayer[i].pitch + global_pitch_decay * (samplePlayer[i].vel) ); /* we have consumed two bytes */
            // samplePlayer[i].samplePosF += 2.0f * sampler_playback * ( samplePlayer[i].pitch + global_pitch_decay * (1-samplePlayer[i].vel) ); /* sounds nice with values for global_pitch_decay > 0 */

            if( samplePlayer[i].pitchdecay > 0.0f ){
              samplePlayer[i].samplePosF += 2.0f * sampler_playback * ( samplePlayer[i].pitch + samplePlayer[i].pitchdecay*samplePlayer[i].vel ); /* we have consumed two bytes */
            }else{
              samplePlayer[i].samplePosF += 2.0f * sampler_playback * ( samplePlayer[i].pitch + samplePlayer[i].pitchdecay*(1-samplePlayer[i].vel) ); /* we have consumed two bytes */
            } 
            // samplePlayer[i].samplePosF += 2.0f * sampler_playback * samplePlayer[i].pitch ; /* we have consumed two bytes */

            if( samplePlayer[i].samplePos >= samplePlayer[i].sampleSize ){
              samplePlayer[i].active = false;
            }
        }
    }

    // PAN?
    *left  = signal_l;
    *right = signal_r;
}
