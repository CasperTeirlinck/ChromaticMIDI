//=============================================================================================
// MIDIUSB Library
//=============================================================================================
#include "MIDIUSB.h"

void noteOn(byte channel, byte pitch, byte velocity){
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity){
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value){
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

//=============================================================================================
// SETUP
//=============================================================================================
#define NUM_ROWS 1 //inputs
#define NUM_COLS 8 //outputs

int note = 36; //lowest note
byte midiCh = 0;
byte midiCC = 1;

const int rowPin[NUM_ROWS] = {10};
const int colPin[NUM_COLS] = {2, 3, 4, 5, 6, 7, 8, 9};

boolean keyPressed[NUM_ROWS][NUM_COLS];
uint8_t keyToMidiMap[NUM_ROWS][NUM_COLS];

unsigned long lastDebounceTime[NUM_ROWS][NUM_COLS] = {0};
unsigned long debounceDelay = 5;

const int potPin = A2;
int potCState = 0;
int potPState = 0;
int potVar = 0;
int potMidiCState = 0;
int potMidiPState = 0;
int potTimeOut = 300;
int potVarThreshold = 10;
boolean potMoving = true;
unsigned long potPTime = 0;
unsigned long potTimer = 0;

void setup(){  

//  for(int rowCtr = 0; rowCtr < NUM_ROWS; ++rowCtr){
//    for(int colCtr = 0; colCtr < NUM_COLS; ++colCtr){
//      keyPressed[rowCtr][colCtr] = false;
//      keyToMidiMap[rowCtr][colCtr] = note;
//      note++;
//    }
//  }
//
//  for (int i = 0; i < NUM_ROWS; ++i) {
//    pinMode(rowPin[i], INPUT_PULLUP);    
//  }
//  for (int i = 0; i < NUM_COLS; ++i) {
//    pinMode(colPin[i], OUTPUT);
//    //digitalWrite(colPin[i], LOW);
//  }
  
//  pinMode(colPin[0], OUTPUT);
//  pinMode(colPin[1], OUTPUT);
//  pinMode(colPin[2], OUTPUT);

//  pinMode(rowPin[0], INPUT_PULLUP);
//  pinMode(rowPin[1], INPUT_PULLUP);
}

//=============================================================================================
// LOOP
//=============================================================================================
void loop(){
//  keys();
  pots();
}

//=============================================================================================
// KEYS
//=============================================================================================
void keys(){
  for (int colCtr = 0; colCtr < NUM_COLS; ++colCtr){

    //scan
    digitalWrite(colPin[colCtr], HIGH);

    int rowValue[NUM_ROWS];

    for (int i = 0; i < NUM_ROWS; ++i) {
      rowValue[i] = !digitalRead(rowPin[i]); // = true when something in optointerrupter
      //Serial.println("col: " + String(colCtr) + "rowval: " + String(digitalRead(rowPin[i])));
    }

    digitalWrite(colPin[colCtr], LOW);
    
    
    // process keys pressed
    for(int rowCtr = 0; rowCtr < NUM_ROWS; ++rowCtr){
      if ( (millis() - lastDebounceTime[rowCtr][colCtr]) > debounceDelay){
        lastDebounceTime[rowCtr][colCtr] = millis();

        if(rowValue[rowCtr] == LOW && !keyPressed[rowCtr][colCtr]){
          keyPressed[rowCtr][colCtr] = true;
          noteOn(midiCh, keyToMidiMap[rowCtr][colCtr], 63);          
          MidiUSB.flush();
        } 
      }
    } 
    
    // process keys released
    for(int rowCtr = 0; rowCtr < NUM_ROWS; ++rowCtr){        
      if(rowValue[rowCtr] == HIGH && keyPressed[rowCtr][colCtr]){
        keyPressed[rowCtr][colCtr] = false;
        noteOff(midiCh, keyToMidiMap[rowCtr][colCtr], 63);
        MidiUSB.flush();
      }
    }
  }
}

//=============================================================================================
// POTS
//=============================================================================================
void pots(){
  potCState = analogRead(potPin);
  potMidiCState = map(potCState, 0, 1023, 127, 0);

  potVar = abs(potCState - potPState);
  if (potVar > potVarThreshold){
    potPTime = millis();
  }
  potTimer = millis() - potPTime;
  if (potTimer < potTimeOut){
    potMoving = true;
  } else {
    potMoving = false;
  }

  if (potMoving == true && potMidiPState != potMidiCState){
    controlChange(midiCh, midiCC + 0, potMidiCState);
    MidiUSB.flush();

    potPState = potCState;
    potMidiPState = potMidiCState;
  }
  
}
