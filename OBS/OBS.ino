#include <Keyboard.h>

//number of buttons in steps of 4; 4 buttons -> 8 LEDs per shiftregister
#define n 16

//keycombinations send for scenes 0 to 9 -> no define needed

//keycombinations send for transitions
#define t1 10
#define t2 11
#define t3 12
#define t4 13

//keycombinations send for live-button
#define stream 14
#define record 15

//keycombinations send for scenes intro/outro
#define intro 16
#define outro 17

//pins for ShiftRegister
#define shiftPin 11
#define storePin 12
#define dataPin  13

//array LED pattern; pattern[NumButtons][2 Bits for color]
int pattern[n][2];

//keykombination ctrl, shift, alt
#define ctrl  128
#define shift 129
#define alt   130

//colors
enum clr { BLK, RED, YEL, GRN };

//states
enum st  { READY, STREAMING, RECORDING };

//buttonPins for scenes
int buttonPin[10]       = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
//buttonPin for LIVE button
int buttonPinLive       = 10;
//buttonPins for transition
int buttonPinSwitch[4]  = {A0, A1, A2, A3};

//variable declaration
int lastLive  = 0;
int live      = 0;
int preview   = 0;
int longPress = 0;
st  state     = READY;

void setup() {
  // shiftRegister -> OUTPUT pins
  pinMode (storePin,  OUTPUT);
  pinMode (shiftPin,  OUTPUT);
  pinMode (dataPin,   OUTPUT);

  // scenes -> INPUT pins
  for (int i = 0; i < 10; i++) {
    pinMode (i, INPUT);
    digitalWrite (i, HIGH);
  }

  // transitions -> INPUT pins
  for (int i = 0; i < 4; i++) {
    pinMode (buttonPinSwitch[i], INPUT);
    digitalWrite (buttonPinSwitch[i], HIGH);
  }

  // LIVE button -> INPUT pin
  pinMode (buttonPinLive, INPUT);
  digitalWrite (buttonPinLive, HIGH);
  
  initColor();
  updateLED();
}

void loop () {
  scanButtons ();
}

void scanButtons () {
  for (int i = 0; i < 10; i++) {                            // scans the scene buttons
    if (digitalRead (buttonPin[i]) == LOW) {
      setPreview (i);
    }
  }
  
  for (int i = 0; i < 4; i++) {                             // scans the transition buttons
    if (digitalRead (buttonPinSwitch[i]) == LOW) {
      switchScene (i);
    }
  }
  
  if (digitalRead (buttonPinLive) == LOW) {                // scans the live button
    liveButtonPressed ();
  }
}


void setPreview(int taste) {
  if (preview == taste && longPress > 0) {
    switchScene(0);
  }
  else {
    longPress++;
    preview = taste;
    keyCom(taste);
    resetColor();
    setColor(live, RED);
    setColor(preview, GRN);
    updateLED();
    delay(500);
  }
}

void switchScene(int uebergang) {
  longPress = 0;
  switch (uebergang) {
    case 0: keyCom(t1); break;         //cut
    case 1: keyCom(t2); break;         //300ms
    case 2: keyCom(t3); break;
    case 3: keyCom(t4); break;
  }

  lastLive = live;                          
  live = preview;
  //setPreview(lastLive);                   //set preview to what was last live
  
  
  int nextPreview = (live + 5) % 10;        //set preview to the same scene with other cam
  setPreview(nextPreview);
  longPress = 0;
}

void resetColor() {                          //set all buttons to color YEL, except LIVE button
  initColor();
  switch (state) {
    case READY:     setColor(14, YEL);  break;
    case STREAMING: setColor(14, GRN);  break;
    case RECORDING: setColor(14, RED);  break;
  }
}

void initColor() {                          //set all buttons to color YEL
  for (int i = n - 1; i >= 0; i--) {
    for (int j = 1; j >= 0; j--) {
      pattern[i][j] = 1;
    }
  }
}

void setColor(int button, clr color) {           //change color of a Button -> updateLED() to activate changes
  switch (color) {
    case RED:     pattern[button][0] = 1;
                  pattern[button][1] = 0; break;
    case YEL:     pattern[button][0] = 1;
                  pattern[button][1] = 1; break;
    case GRN:     pattern[button][0] = 0;
                  pattern[button][1] = 1; break;
    case BLK:     pattern[button][0] = 0;
                  pattern[button][1] = 0; break;
  }
}

void updateLED() {                            //make color changes visible; push pattern on shiftregisters
  // storePin sicherheitshalber auf LOW
  digitalWrite(storePin, LOW);

  for (int i = n - 1; i >= 0; i--) {
    for (int j = 1; j >= 0; j--) {
      // Zuerst immer alle 3 Pins auf LOW
      // Aktion passiert bei Wechsel von LOW auf HIGH
      digitalWrite(shiftPin, LOW);
      // Jetzt den Wert der aktuellen Stelle ans Datenpin DS anlegen
      digitalWrite(dataPin, !pattern[i][j]);
      // Dann ShiftPin SHCP von LOW auf HIGH, damit wird der Wert
      // am Datenpin ins Register geschoben.
      digitalWrite(shiftPin, HIGH);
    }
  }

  // Wenn alle 8 Stellen im Register sind, jetzt das StorePin STCP
  // von LOW auf HIGH, damit wird Registerinhalt an Ausgabepins
  // kopiert und der Zustand an den LEDs sichtbar
  digitalWrite(storePin, HIGH);
}

void liveButtonPressed() {
  delay(100);
  //switch mit state variable - Stream, record, Aus
  switch (state) {
    case READY:     state = STREAMING;
                    keyCom(stream);       //start stream
                    keyCom(intro);        //set scene intro
                    setColor(14, GRN);    //LIVE button green
                    updateLED();          break;
              
    case STREAMING: state = RECORDING;
                    keyCom(record);       //start recording
                    setColor(14, RED);    //LIVE button red
                    updateLED();          break;
              
    case RECORDING: setColor(12, GRN);    //YES
                    setColor(13, RED);    //NO
                    updateLED();
                    delay(100);
                    for (;;) {
                      //STOP?, if YES: -> scene outro, stop recording, wait, stop streaming
                      if (digitalRead(buttonPinSwitch[2]) == LOW) {
                        stop();
                        break;
                      }
                      //if NO: -> cancel
                      else if (digitalRead(buttonPinSwitch[3]) == LOW) {
                      setColor(12, YEL);
                      setColor(13, YEL);
                      updateLED();
                      delay(300);
                      break;
                      }
                    }
  }
}

void keyCom (int x) {   //handles the keycombinations
  Keyboard.press(ctrl);
  Keyboard.press(shift);
  Keyboard.press(alt);
  
  switch(x){
    case 0:       Keyboard.press('0');
                    break;
    case 1:       Keyboard.press('1');
                    break;
    case 2:       Keyboard.press('2');
                    break;
    case 3:       Keyboard.press('3');
                    break;
    case 4:       Keyboard.press('4');
                    break;
    case 5:       Keyboard.press('5');
                    break;
    case 6:       Keyboard.press('6');
                    break;
    case 7:       Keyboard.press('7');
                    break;
    case 8:       Keyboard.press('8');
                    break;
    case 9:       Keyboard.press('9');
                    break;
    case t1:      Keyboard.press('q');
                    break;
    case t2:      Keyboard.press('w');
                    break;
    case t3:      Keyboard.press('e');
                    break;
    case t4:      Keyboard.press('r');
                    break;
    case stream:  Keyboard.press('t');
                    break;
    case record:  Keyboard.press('z');
                    break;
    case intro:   Keyboard.press('i');
                    break;
    case outro:   Keyboard.press('o');
                    break;
  }
  delay(100);
  Keyboard.releaseAll();
}

void stop() {      //stop recording, wait, stop streaming, update LIVE button
  //stop recording
  keyCom(record);
  keyCom(outro);
  
  //wait
  for(int i = 0; i < 300; i++) {
    if(digitalRead(buttonPinLive) == LOW) {      //stop stream immediatly
      break;
    }
    delay(100);
  }

  //stop streaming
  keyCom(stream);

  //update LIVE button
  state = READY;
  setColor(14, YEL);
  updateLED();
  longPress = 0;
  delay(200);
}

