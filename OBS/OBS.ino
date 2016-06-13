#include <Keyboard.h>

//Anzahl ShiftRegister fuer LED in 4er Schritten
#define n 4

//Tastenkombinationen Uebergaenge
#define t1 10
#define t2 11
#define t3 12
#define t4 13

//Tastenkombinationen Live
#define stream 14
#define aufnahme 15

//Pins fuer das ShiftRegister
#define shiftPin 11
#define storePin 12
#define dataPin  13

//Tastenarray fuer LED
int muster[4][2];

//Tastenkombination Ctrl
#define ctrl  128
#define shift 129
#define alt   130

//Farben festlegen
enum frb { AUS, ROT, GELB, GRUEN };

//TastenPins Szene
int tastenPin[10]       = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
//TastenPin Live
int tastenPinLive       = 10;
//TastenPins Switch
int tastenPinSwitch[4]  = {A0, A1, A2, A3};

//Variablen
int lastLive = 0;
int live = 0;
int preview = 0;
int streaming = 0;
int longPress = 0;

void setup() {
  // ShiftRegister
  pinMode(storePin, OUTPUT);
  pinMode(shiftPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  //PINMODE FUER UEBRIGE PINS FESTLEGEN!!!!
  initFarbe();
}

void loop () {



  resetFarbe();
  updateLED();
  delay(1000);
  setFarbe(0, ROT);
  setFarbe(1, ROT);
  setFarbe(2, ROT);
  setFarbe(3, ROT);
  updateLED();
  delay(1000);
  setFarbe(0, GRUEN);
  setFarbe(1, GRUEN);
  setFarbe(2, GRUEN);
  setFarbe(3, GRUEN);
  updateLED();
  delay(1000);
}

void tastenAuslesen() {
  for (int i = 0; i < 10; i++) {
    if (digitalRead(tastenPin[i]) == HIGH) {
      setPreview(i);
    }
  }
  for (int i = 0; i < 4; i++) {
    if (digitalRead(tastenPinSwitch[i]) == HIGH) {
      switchScene(i);
    }
  }
  if (digitalRead(tastenPinLive) == HIGH) {
    liveButtonPressed();
  }
}


void setPreview(int taste) {
  if (preview == taste && longPress > 0) {
    switchScene(preview);
  }
  else {
    longPress++;
    preview = taste;
    tastenKombi(taste);
    resetFarbe();
    setFarbe(live, ROT);
    setFarbe(preview, GRUEN);
    updateLED();
    delay(500);
  }
}

void switchScene(int uebergang) {
  switch (uebergang) {
    case 0: tastenKombi(t1); break;         //cut
    case 1: tastenKombi(t2); break;         //300ms
    case 2: tastenKombi(t3); break;
    case 3: tastenKombi(t4); break;
  }

  lastLive = live;                          
  live = preview;
  //setPreview(lastLive);                   //setzt preview auf das was zuletzt live war
  
  
  int nextPreview = (live + 5) % 10;        //setzt preview auf gleiche Szene mit anderer Cam
  setPreview(nextPreview);

  longPress = 0;
}

void resetFarbe() {                          //setzt alle Knöpfe auf GELB, ausser LIVE Button
  for (int i = n - 3; i >= 0; i--) {
    for (int j = 1; j >= 0; j--) {
      muster[i][j] = 1;
    }
  }
}

void initFarbe() {                          //setzt alle Knöpfe auf GELB
  for (int i = n - 1; i >= 0; i--) {
    for (int j = 1; j >= 0; j--) {
      muster[i][j] = 1;
    }
  }
}

void setFarbe(int button, frb farbe) {           //farbe einer Taste aendern -> danach updateLED() aufrufen
  switch (farbe) {
    case ROT:     muster[button][0] = 1;
      muster[button][1] = 0; break;
    case GELB:    muster[button][0] = 1;
      muster[button][1] = 1; break;
    case GRUEN:   muster[button][0] = 0;
      muster[button][1] = 1; break;
    case AUS:     muster[button][0] = 1;
      muster[button][1] = 1; break;
  }
}

void updateLED() {                            //Farbaenderungen sichtbar machen
  // storePin sicherheitshalber auf LOW
  digitalWrite(storePin, LOW);

  for (int i = n - 1; i >= 0; i--) {
    for (int j = 1; j >= 0; j--) {
      // Zuerst immer alle 3 Pins auf LOW
      // Aktion passiert bei Wechsel von LOW auf HIGH
      digitalWrite(shiftPin, LOW);
      // Jetzt den Wert der aktuellen Stelle ans Datenpin DS anlegen
      digitalWrite(dataPin, !muster[i][j]);
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
  //switch mit streaming variable - Stream, Aufnahme, Aus
  switch (streaming) {
    case 0:   streaming++;
              tastenKombi(stream);  
              setFarbe(16, GRUEN);  
              updateLED();          break;    //Starte Stream ohne Aufnahme, Szene Intro
    case 1:   streaming++;
              tastenKombi(aufnahme);
              setFarbe(16, ROT);    
              updateLED();          break;    //Starte Aufnahme
    case 2:   setFarbe(12, GRUEN);
              setFarbe(13, ROT);
              updateLED();
              for (;;) {
                //Bestätigung abfragen, wenn ja: Szene Outro, Aufnahme beenden, 10 Minuten warten, Stream beenden
                if (digitalRead(tastenPinSwitch[2]) == HIGH) {
                  ende();
                  break;
                }
                //wenn nicht abbrechen
                else if (digitalRead(tastenPinSwitch[3]) == HIGH) {
                setFarbe(12, GELB);
                setFarbe(13, GELB);
                break;
                }
              }
  }
}

void tastenKombi (int x) {
  
  Keyboard.press(ctrl);
  Keyboard.press(shift);
  Keyboard.press(alt);
  if(x < 10){
    Keyboard.press(x);
  }
  else{
    switch(x){
      case t1:  Keyboard.press('q');
                break;
      case t2:  Keyboard.press('w');
                break;
      case t3:  Keyboard.press('e');
                break;
      case t4:  Keyboard.press('r');
                break;
      case stream:  Keyboard.press('t');
                break;
      case aufnahme:  Keyboard.press('z');
                break;
    }
  }
  delay(100);
  Keyboard.releaseAll();
}

void ende() {                                     //Aufnahme beenden, 10 min warten, stream beenden, Controller resetten
  
  //Aufnahme beenden
  tastenKombi(aufnahme);

  //Warten
  for(int i = 0; i < 300; i++) {
    if(digitalRead(tastenPinLive) == HIGH) {      //Stream sofort beenden
      break;
    }
    delay(100);
  }

  //Reset
  tastenKombi(stream);
  initFarbe();
  lastLive  = 0;
  live      = 0;
  preview   = 0;
  streaming = 0;
  longPress = 0;
}

