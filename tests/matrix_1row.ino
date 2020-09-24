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
#define NUM_COLS 8 //outputs

const int rowPin = 10;
const int colPin[NUM_COLS] = {2, 3, 4, 5, 6, 7, 8, 9};

int rowVal;

unsigned long lastDebounceTime[NUM_COLS] = {0};
unsigned long debounceDelay = 5;

int note = 36; //lowest note
byte midiCh = 0;

boolean keyPressed[NUM_COLS];
uint8_t keyToMidiMap[NUM_COLS];

void setup() {

  pinMode(rowPin, INPUT_PULLUP);
  
  for (int col = 0; col < NUM_COLS; ++col) {
    pinMode(colPin[col], OUTPUT);
    digitalWrite(colPin[col], LOW);
  }

  for(int col = 0; col < NUM_COLS; ++col){
    keyPressed[col] = false;
    keyToMidiMap[col] = note;
    note++;
  }

}

//=============================================================================================
// LOOP
//=============================================================================================
void loop() {

  keys();

}

//=============================================================================================
// KEYS
//=============================================================================================
void keys() {

  for (int col = 0; col < NUM_COLS; ++col) {      
  
      digitalWrite(colPin[col], HIGH);
      delay(1);
      rowVal = digitalRead(rowPin);
      digitalWrite(colPin[col], LOW);
    
      if (rowVal == HIGH && !keyPressed[col]) {
        if ( (millis() - lastDebounceTime[col]) > debounceDelay){
          lastDebounceTime[col] = millis();

          keyPressed[col] = true;
          noteOn(midiCh, keyToMidiMap[col], 63);
          MidiUSB.flush();
        }
      }
      if (rowVal == LOW && keyPressed[col]) {
        keyPressed[col] = false;
        noteOff(midiCh, keyToMidiMap[col], 63);
        MidiUSB.flush();
      }

//      if ( (millis() - lastDebounceTime[col]) > debounceDelay){
//          lastDebounceTime[col] = millis();
//
//          if (rowVal == HIGH) {
//            noteOn(midiCh, note, 63);          
//            MidiUSB.flush();
//          }
//      }      
      
    }
    
}
