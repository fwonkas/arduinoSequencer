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

unsigned int slotOrder[] = {0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
volatile unsigned int slot = 0;
unsigned int selSlot = 0;
volatile unsigned int steps[] = {40,80,160,320,40,80,160,320,40,80,160,320,40,80,160,320};
unsigned int rotaryPins[] = {Rotary1, Rotary2, Rotary4, Rotary8};
unsigned int numSteps = sizeof(steps) / sizeof(int);
unsigned int duration = 50;
unsigned int pitchval = 40;
unsigned int tempo = 100;
unsigned int frequency = 0;
unsigned long displayInterval = millis();

SPI_VFD vfd(5, 6, 7);

void setup()
{
  pinMode (DigitalOutSignal, OUTPUT);
  attachInterrupt(0, assignFreq, RISING);
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
}

void loop() {
  
  frequency = map(analogRead(FrequencyIn), 0, 1023, 20, 2047);
  
  for (int i = 0; i < numSteps; i++) {
    tempo = analogRead(TempoIn);
    duration = map(analogRead(DurationIn), 0, 1023, 1, tempo);
    digitalWrite(LED, HIGH);
    tone(DigitalOutSignal, steps[i], duration);
    digitalWrite(LED, LOW);
    unsigned int val = 0;
    unsigned int inDec = 0;
    slot = 0;
    vfd.clear();
    displayInterval = millis();
    for (int j = 0; j < 4; j++) {
      val = digitalRead(rotaryPins[j]);
      inDec = val << j;
      slot += (inDec);
    }
    vfd.setCursor(0,0);
    vfd.print("Slot: ");
    vfd.print(slotOrder[slot]);
    vfd.print(":");
    vfd.print(i);
    vfd.setCursor(13, 0);
    vfd.print("F: ");
    vfd.print(map(analogRead(FrequencyIn), 0, 1023, 40, 2047));
    vfd.setCursor(0,1);
    vfd.print("D: ");
    vfd.print(duration);
    vfd.setCursor(13, 1);
    vfd.print("T: ");
    vfd.print(tempo);
    delay(tempo);
  }

}

void assignFreq() {
  unsigned int newSlot = 0;
  unsigned int i = 0;
  unsigned int val = 0;
  unsigned int inDec = 0;
  for (; i < 4; i++) {
    val = digitalRead(rotaryPins[i]);
    inDec = val << i;
    newSlot += (inDec);
  }
  slot = newSlot;
  unsigned int frq = map(analogRead(FrequencyIn), 0, 1023, 40, 2047);
  steps[slotOrder[slot]] = frq;
}
