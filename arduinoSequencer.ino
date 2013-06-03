#include <SPI_VFD.h>

#define Version 0.1
#define DurationIn A0
#define FrequencyIn A1
#define TempoIn A2
#define Rotary1 8
#define Rotary2 12
#define Rotary4 10
#define Rotary8 11
#define ENC_PORT PINC
#define DigitalOutSignal 9

#define AssignFrequency 2
#define LED 13

#define NumReadings 10

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
unsigned int timeLeft = 0;

unsigned int durations[NumReadings];
unsigned int tempos[NumReadings];
unsigned int frequencies[NumReadings];

unsigned int totalDurations = 0;
unsigned int totalTempos = 0;
unsigned int totalFrequencies = 0;

unsigned int index = 0;

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
  vfd.setCursor(0,0);
  vfd.print("Arduino Sequencer v");
  vfd.print(Version);
  delay(3000);
  vfd.clear();
}

void loop() {
  getPots();
  updateDisplay();
  digitalWrite(LED, HIGH);
  tone(DigitalOutSignal, steps[currentSlot], duration);
  digitalWrite(LED, LOW);
  nextSlot = currentSlot + 1;
  currentSlot = (nextSlot == numSteps) ? 0 : nextSlot;
  timeLeft = tempo;
  while (timeLeft > 100) {
    getPots();
    updateDisplay();
    timeLeft -= 100;
    delay(100);
  }
  if (timeLeft > 0) {
    delay(timeLeft);
  }
}

void getPots() {
  totalDurations -= durations[index];
  totalTempos -= tempos[index];
  totalFrequencies -= frequencies[index];
  durations[index] = roundDown(map(analogRead(DurationIn), 0, 1023, 1020, 0));
  tempos[index] = roundDown(map(analogRead(TempoIn), 0, 1023, 0, 1000));
  frequencies[index] = roundDown(map(analogRead(FrequencyIn), 0, 1023, 0, 2047));
  totalDurations += durations[index];
  totalTempos += tempos[index];
  totalFrequencies += frequencies[index];
  index += 1;
  if (index >= NumReadings) {
    index = 0;
  }
  duration = max(10, roundDown(totalDurations / NumReadings));
  tempo = roundDown(totalTempos / NumReadings);
  frequency = roundDown(totalFrequencies / NumReadings);
  //frequency = roundDown(map(analogRead(FrequencyIn), 0, 1020, 0, 2047));
  //tempo = roundDown(map(analogRead(TempoIn), 0, 1023, 0, 1000));
  //duration = roundDown(map(analogRead(DurationIn), 0, 1023, 1023, 10));
}

void updateDisplay() {
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
  /**
    * This may look a little confusing. All it's doing is checking each
    * rotary pin. In position 0, it's grabbing the value (0 or 1)
    * and multiplying it by 2^0. In position 1, it's multiplying by 2^1,
    * and so on.
    * It could be more clearly written as:
    * 
    * for (; position < 4; position++) {
    *   value += digitalRead(rotaryPins[position]) * pow(2, position);
    * }
    * 
    * But using bitshifting is faster.
    */

  for (; position < 4; position++) {
    value += digitalRead(rotaryPins[position]) << position;
  }
  return value;
}

String padding(int number, byte width) {
  /**
    * Pad a number as a string.
    */
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
  /**
    * This is the equivalent to:
    *   return (x * 10);
    */
  return ((x << 3) + (x << 1));
}
