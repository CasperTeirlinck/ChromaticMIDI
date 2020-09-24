#include "MIDIUSB.h"

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

const int NButtons = 2;
const int buttonPin[NButtons] = {14, 15};
int buttonCState[NButtons] = {0};
int buttonPState[NButtons] = {0};

unsigned long lastDebounceTime[NButtons]  = {0};
unsigned long debounceDelay = 5;

byte midiCh = 1;
byte note = 36;

void setup() {

  for ( int i = 0; i < NButtons; i++ ){
    pinMode(buttonPin[i], INPUT_PULLUP);
  }
  
}

void loop() {

  buttons();
  
}

void buttons(){

  for (int i = 0; i < NButtons; i++){
      buttonCState[i] = digitalRead(buttonPin[i]);
      if ( (millis() - lastDebounceTime[i]) > debounceDelay ){
          if ( buttonPState[i] != buttonCState[i] ){
            lastDebounceTime[i] = millis();
            if ( buttonCState[i] == LOW ){
              noteOn(midiCh, note+i, 127);
              MidiUSB.flush();
            }
            else {
              noteOff(midiCh, note+i, 127);
              MidiUSB.flush();
            }
            buttonPState[i] = buttonCState[i];
          }  
      }
  }
  
}
