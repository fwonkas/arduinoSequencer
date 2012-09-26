#include <SPI_VFD.h>
#include <TimerOne.h>

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
unsigned int currentSlot = 0;
unsigned int nextSlot;
volatile unsigned int slot = 0;
volatile unsigned int steps[] = {40,80,160,320,40,80,160,320,40,80,160,320,40,80,160,320};
unsigned int rotaryPins[] = {Rotary1, Rotary2, Rotary4, Rotary8};
unsigned int numSteps = sizeof(steps) / sizeof(int);
unsigned int duration = 1023;
unsigned int tempo = 0;
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
  updateDisplay();
  Timer1.initialize(50000/3);
  Timer1.attachInterrupt(updateDisplay);
}

void loop() {
  digitalWrite(LED, HIGH);
  tone(DigitalOutSignal, steps[currentSlot], duration);
  digitalWrite(LED, LOW);
  nextSlot = currentSlot + 1;
  currentSlot = (nextSlot == numSteps) ? 0 : nextSlot;
  delay(tempo);
}

void updateDisplay() {
  frequency = roundDown(map(analogRead(FrequencyIn), 0, 1020, 0, 2047));
  tempo = roundDown(map(analogRead(TempoIn), 0, 1023, 0, 1000));
  duration = roundDown(map(analogRead(DurationIn), 0, 1023, 1023, 10));
  slot = getRotaryValue();
  vfd.setCursor(0,0);
  vfd.print("Slot: ");
  vfd.print(padding(slotOrder[slot], 2));
  vfd.print(":");
  vfd.print(padding(currentSlot, 2));
  vfd.setCursor(13, 0);
  vfd.print("F: ");
  vfd.print(padding(frequency, 4));
  vfd.setCursor(0,1);
  vfd.print("D: ");
  vfd.print(padding(duration, 4));
  vfd.setCursor(13, 1);
  vfd.print("T: ");
  vfd.print(padding(tempo, 4));
}

void assignFreq() {
  slot = getRotaryValue();
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
  return multByTen(x / 10);
}

unsigned int multByTen(unsigned int x) {
  return ((x << 3) + (x << 1));
}