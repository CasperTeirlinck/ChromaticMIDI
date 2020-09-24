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
#define NUM_ROWS 2 //inputs
#define NUM_COLS 8 //outputs

const int rowPin[NUM_ROWS] = {10, 16};
const int colPin[NUM_COLS] = {2, 3, 4, 5, 6, 7, 8, 9};

int rowVal[NUM_ROWS];

unsigned long lastDebounceTime[NUM_ROWS][NUM_COLS] = {0};
unsigned long debounceDelay = 5;

int note = 36; //lowest note
byte midiCh = 0;

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

      // read pins
      digitalWrite(colPin[col], HIGH);
      delay(1);
      for (int row = 0; row < NUM_ROWS; ++row) {
        rowVal[row] = digitalRead(rowPin[row]);
      }
      digitalWrite(colPin[col], LOW);

      // note on
      for (int row = 0; row < NUM_ROWS; ++row) {
        if (rowVal[row] == HIGH && !keyPressed[row][col]) {
          if ((millis() - lastDebounceTime[row][col]) > debounceDelay){
            lastDebounceTime[row][col] = millis();
  
            keyPressed[row][col] = true;
            noteOn(midiCh, keyToMidiMap[row][col], 63);
            MidiUSB.flush();
          }
        }
      }
      
      // note off
      for (int row = 0; row < NUM_ROWS; ++row) {
        if (rowVal[row] == LOW && keyPressed[row][col]) {
          keyPressed[row][col] = false;
          noteOff(midiCh, keyToMidiMap[row][col], 63);
          MidiUSB.flush();
        }
      }     
      
    }
    
}
