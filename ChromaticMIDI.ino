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

// LAYOUT:
// usb *top*
// r1
// r2
//
//
// r3 --- btns
// r4 --- slider
// r5 --- c8
// r6 --- c7
// r7 --- c6
// r8 --- c5
// c1 --- c4
// c2 --- c3

//#define NUM_ROWS 8 //inputs
#define NUM_ROWS 2 //inputs
#define NUM_COLS 8 //outputs

const int rowPin[NUM_ROWS] = {6, 7};
const int colPin[NUM_COLS] = {8, 9, 10, 16, 14, 15, 18, 19};

int rowVal[NUM_ROWS];

unsigned long lastDebounceTime[NUM_ROWS][NUM_COLS] = {0};
unsigned long debounceDelay = 2;

const int btnsPin = A3;
int btnsCounter = 0;
long btnsTime = 0;
int btnsDebounceCount = 5;
int btnsCurrentState = 0;
int btnsVal;

const int potPin = A2;
byte potMode = 0;
int potCState = 0;
int potPState = 0;
int potVar = 0;
int potMidiCState = 0;
int potMidiPState = 0;
int potVelCState = 0;
int potVelPState = 0;
int potTimeOut = 300;
int potVarThreshold = 10;
boolean potMoving = true;
unsigned long potPTime = 0;
unsigned long potTimer = 0;

int note = 43; //lowest note
byte velocity = 63;
byte midiCh = 0;
byte midiCC = 1;

boolean keyPressed[NUM_ROWS][NUM_COLS];
uint8_t keyToMidiMap[NUM_ROWS][NUM_COLS];

void setup() {

  for (int row = 0; row < NUM_ROWS; ++row) {
    pinMode(rowPin[row], INPUT_PULLUP);
  }
  
  for (int col = 0; col < NUM_COLS; ++col) {
    pinMode(colPin[col], OUTPUT);
    digitalWrite(colPin[col], LOW);
  }

  for(int row = 0; row < NUM_ROWS; ++row){
    for (int col = 0; col < NUM_COLS; ++col) {
      keyPressed[row][col] = false;
      keyToMidiMap[row][col] = note;
      note++;
    }
  }

  pinMode(btnsPin, INPUT_PULLUP);

}

//=============================================================================================
// LOOP
//=============================================================================================
void loop() {

  keys();

  pots();

  btns();

}

//=============================================================================================
// KEYS
//=============================================================================================
void keys() {

  for (int col = 0; col < NUM_COLS; ++col) {      

      // read pins
      digitalWrite(colPin[col], HIGH);
      delay(1);
      for (int row = 0; row < NUM_ROWS; ++row) {
        rowVal[row] = digitalRead(rowPin[row]);
      }
      digitalWrite(colPin[col], LOW);

      // note on
      for (int row = 0; row < NUM_ROWS; ++row) {
        if (rowVal[row] == LOW && !keyPressed[row][col]) {
          if ((millis() - lastDebounceTime[row][col]) > debounceDelay){
            lastDebounceTime[row][col] = millis();
  
            keyPressed[row][col] = true;
            noteOn(midiCh, keyToMidiMap[row][col], velocity);
            MidiUSB.flush();
          }
        }
      }
      
      // note off
      for (int row = 0; row < NUM_ROWS; ++row) {
        if (rowVal[row] == HIGH && keyPressed[row][col]) {
          keyPressed[row][col] = false;
          noteOff(midiCh, keyToMidiMap[row][col], velocity);
          MidiUSB.flush();
        }
      }     
      
    }
    
}

//=============================================================================================
// POTS
//=============================================================================================
void pots() {
  potCState = analogRead(potPin);
  potMidiCState = map(potCState, 0, 1023, 127, 0);
  potVelCState = map(potCState, 0, 1023, 127, 0);

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

  if (potMode == 0 && potMoving == true && potMidiPState != potMidiCState){ 
    controlChange(midiCh, midiCC + 0, potMidiCState);
    MidiUSB.flush();

    potPState = potCState;
    potMidiPState = potMidiCState;
  }

  if (potMode == 1 && potMoving == true && potVelPState != potVelCState){
    velocity =  potVelCState;

    potPState = potCState;
    potVelPState = potVelCState;
  }
  
}

//=============================================================================================
// BTNS
//=============================================================================================
void btns() {
//  Serial.println(analogRead(btnsPin));
//  delay(100);
  if (millis() != btnsTime) {
    btnsVal = analogRead(btnsPin);
    if (btnsVal == btnsCurrentState && btnsCounter > 0) {
      btnsCounter--;
    }
    if (btnsVal != btnsCurrentState) {
      btnsCounter++;
    }
    if (btnsCounter >= btnsDebounceCount) {
      btnsCounter = 0;
      btnsCurrentState = btnsVal;
      if (btnsVal > 0) {
        // L: 73 - 76
        // R: 41 - 44
        // U: 52 - 56 || 36 - 39
        // D: 19 - 23
        if (btnsVal >= 73 && btnsVal <= 76) {
          potMode = 0;
        }
        if (btnsVal >= 41 && btnsVal <= 44) {
          potMode = 1;
        }
        if ((btnsVal >= 52 && btnsVal <= 56) || (btnsVal >= 36 && btnsVal <= 39)) {
          changeOctave(true);
        }
        if (btnsVal >= 19 && btnsVal <= 23) {
          changeOctave(false);
        }
        
      }
    }
    btnsTime = millis();
  }

}

void changeOctave(bool up) {  
  for(int row = 0; row < NUM_ROWS; ++row){
    for (int col = 0; col < NUM_COLS; ++col) {
      if (up) {
        keyToMidiMap[row][col] = keyToMidiMap[row][col] + 12; 
      } else {
        keyToMidiMap[row][col] = keyToMidiMap[row][col] - 12; 
      }
    }
  }
}
