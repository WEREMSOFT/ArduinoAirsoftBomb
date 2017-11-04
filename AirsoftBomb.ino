
#include <Arduino.h>
#include <TM1637Display.h>
#include "pitches.h"


// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3

// The amount of time (in milliseconds) between tests
#define TEST_DELAY   1000

const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};

const uint8_t SEG_MINUS[] = {
  SEG_G,
  SEG_G,
  SEG_G,
  SEG_G
  };

// notes in the melody:
int melody[] = {
  NOTE_G3, NOTE_A3, NOTE_G3, NOTE_A3, NOTE_G3, NOTE_A3, NOTE_G3, NOTE_A3
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  2, 2, 2, 2, 2, 2, 2, 2
};

TM1637Display display(CLK, DIO);
int minutes = 0;
int seconds = 0;
int countdown = 5;
int brigtness = 0xff;
uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
const int buttonPin = 7;
const int pilotLightPin = 11;
// variables will change:
int buttonState = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

// Gameplay Related Variables
const int TICKS_TO_ACTIVATE_DEACTIVATE = 10;
int ticksCounter = 0;
bool pilotLedState = false;
const int SECONDS_TILL_EXPLODE = 120;

// State Handling
const int STATE_DEACTIVATED = 0;
const int STATE_RUNNING = 1;
const int STATE_ACTIVATING = 2;
const int STATE_DEACTIVATING = 3;
const int STATE_EXPLODING = 4;
const int STATE_IDLE = 5;

int state = STATE_DEACTIVATED;  
void setup()
{
  Serial.begin(9600); 
  display.setBrightness(0x0f);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  pinMode(pilotLightPin, OUTPUT);
  digitalWrite(pilotLightPin, HIGH);
  previousMillis = millis();
  passToStateIdle();
}

void loop()
{
  buttonState = digitalRead(buttonPin);
  currentMillis = millis();
  ledHandler();
  if (currentMillis - previousMillis >= TEST_DELAY) {
    tickHandler();
    switch(state){
      case STATE_IDLE:
        processStateIdle();
        break;
      
      case STATE_DEACTIVATED:
        processStateDeactivated();
        break;
      
      case STATE_RUNNING:
        processStateRunning();
        break;
      
      case STATE_EXPLODING:
        processStateExploding();
        break;
    }
    previousMillis = currentMillis;
  }
}
unsigned long ledmillis = millis();
unsigned long currentledmillis = 0;
void ledHandler(){
    if(ticksCounter == 0) {
      digitalWrite(pilotLightPin, LOW);
      return;
    }
    currentledmillis = millis();
    if (currentledmillis - ledmillis >= 250) {
      pilotLedState = !pilotLedState;
      digitalWrite(pilotLightPin, pilotLedState?HIGH:LOW);
      tone(8, NOTE_GS6, 100);
      ledmillis = currentledmillis;
    }
}

void passToStateIdle(){
  display.setBrightness(0x0f);
  ticksCounter = 0;
  state = STATE_IDLE;
  // Done!
  display.setSegments(SEG_MINUS);
}

void processStateIdle(){
  if(ticksCounter >= TICKS_TO_ACTIVATE_DEACTIVATE)  passToStateRunning();
}

void passToStateDeactivated(){
  ticksCounter = 0;
  state = STATE_DEACTIVATED;
  // Done!
  display.setSegments(SEG_DONE);
}

void processStateDeactivated(){
     if(ticksCounter >= TICKS_TO_ACTIVATE_DEACTIVATE)  passToStateRunning();
}




void processStateExploding(){
  if(ticksCounter >= TICKS_TO_ACTIVATE_DEACTIVATE) {
    passToStateIdle();
    return;
  }
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(8, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(100);
    // stop the tone playing:
    noTone(8);

  }
}

void passToStateRunning(){
  ticksCounter = 0;
  countdown = SECONDS_TILL_EXPLODE;
  state = STATE_RUNNING;
}

void processStateRunning(){
  if(ticksCounter >= TICKS_TO_ACTIVATE_DEACTIVATE) {
    passToStateDeactivated();
    return;
  }
  if(countdown == 0)
    passToStateExploding();
 // read the state of the pushbutton value:
  Serial.println(buttonState);
  tone(8, NOTE_DS8, 200);
  minutes = countdown / 60;
  seconds = countdown % 60;
  display.showNumberDecEx(minutes * 100 + seconds, 64, true);
  countdown--;

}

void passToStateExploding(){
  state = STATE_EXPLODING;
  tone(8, NOTE_B0, 2000);
}

void tickHandler(){
  if(buttonState == 1) {
    ticksCounter++;
  }else{
    ticksCounter = 0;
  }
}

