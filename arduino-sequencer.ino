#include <SPI_VFD.h>

#define DurationIn A0
#define FrequencyIn A1
#define TempoIn A2
#define Rotary1 8
#define Rotary2 9
#define Rotary4 10
#define Rotary8 11
#define ENC_PORT PINC
#define DigitalOutSignal 12

#define AssignFrequency 2
#define LED 13

unsigned int slotOrder[] = {7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8};
volatile unsigned int slot = 0;
unsigned int selSlot = 0;
volatile unsigned int steps[] = {40,80,160,320,40,80,160,320,40,80,160,320,40,80,160,320};
unsigned int rotaryPins[] = {Rotary1, Rotary2, Rotary4, Rotary8};
unsigned int numSteps = sizeof(steps) / sizeof(int);
unsigned int duration = 50;
unsigned int pitchval = 40;
unsigned int tempo = 100;
unsigned int frequency = 0;

SPI_VFD vfd(5, 6, 7);

void setup() {
  pinMode (DigitalOutSignal, OUTPUT);
  attachInterrupt(0, assignFreq, RISING);
  pinMode (Rotary1,INPUT);
  pinMode (Rotary2,INPUT);
  pinMode (Rotary4,INPUT);
  pinMode (Rotary8,INPUT);
  pinMode (LED, OUTPUT);
  Serial.begin(9600);
  vfd.noDisplay();
  delay(500);
   // Turn on the display:
  vfd.display();
  // set up the VFD's number of columns and rows: 
  vfd.begin(20, 2);
  vfd.clear();
}

void loop() {
  for (int i = 0; i < numSteps; i++) {
    frequency = roundDown(map(analogRead(FrequencyIn), 0, 1020, 0, 2047));
    tempo = roundDown(map(analogRead(TempoIn), 0, 1023, 0, 1000));
    duration = roundDown(map(analogRead(DurationIn), 0, 1023, 1023, 10));
    digitalWrite(LED, HIGH);
    tone(DigitalOutSignal, steps[i], duration);
    digitalWrite(LED, LOW);
    slot = getRotaryValue();
    vfd.setCursor(0,0);
    vfd.print("Slot: ");
    vfd.print(padding(slotOrder[slot], 2));
    vfd.print(":");
    vfd.print(padding(i, 2));
    vfd.setCursor(13, 0);
    vfd.print("F: ");
    vfd.print(padding(frequency, 4));
    vfd.setCursor(0,1);
    vfd.print("D: ");
    vfd.print(padding(duration, 4));
    vfd.setCursor(13, 1);
    vfd.print("T: ");
    vfd.print(padding(tempo, 4));
    delay(tempo);
  }
}

void assignFreq() {
  slot = getRotaryValue();
//  unsigned int frq = map(analogRead(FrequencyIn), 0, 1023, 40, 2047);
  unsigned int frq = roundDown(map(analogRead(FrequencyIn), 0, 1020, 0, 2047));
  steps[slotOrder[slot]] = frq;
}

unsigned int getRotaryValue() {
  unsigned int position = 0;
  unsigned int value = 0;
  for (; position < 4; position++) {
    value += digitalRead(rotaryPins[position]) << position;
  }
  return value;
}

String padding(int number, byte width) {
  String value;
  value = String("");
  int currentMax = 10;
  for (byte i=1; i < width; i++){
    if (number < currentMax) {
      value = String("0" + value);
    }
    currentMax *= 10;
  }
  return String(value + number);
}

unsigned int roundDown(unsigned int x) {
  // This is a psychotic way to round an integer down to the nearest 10.
  // It's also inaccurate starting at x == 1029, but we're only using values
  // up to 1000 here.
  // The idea here is to waste as little time as possible.
  return multByTen(divByTen(x));
}

unsigned int divByTen(unsigned int x) {
  return ((x << 7) + (x << 6) + (x << 3) + (x << 2) + (x << 0)) >> 11;
}

unsigned int multByTen(unsigned int x) {
  return ((x << 3) + (x << 1));
}
