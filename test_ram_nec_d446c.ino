#include "Arduino.h"

#define RAM_IO1  4
#define RAM_IO2  5
#define RAM_IO3  6
#define RAM_IO4  7
#define RAM_IO5  8
#define RAM_IO6  9
#define RAM_IO7  10
#define RAM_IO8  11

#define RAM_A0  A0
#define RAM_A1  A1
#define RAM_A2  A2
#define RAM_A3  A3
#define RAM_A4  A4
#define RAM_A5  A5

#define RAM_WE  2
#define RAM_OE  3
#define RAM_CS  12

#define RAM_SELECT_HIGH_ADD 13

void setDatabusOut(bool isOut) {
  if (isOut) {
    pinMode(RAM_IO1, OUTPUT);
    pinMode(RAM_IO2, OUTPUT);
    pinMode(RAM_IO3, OUTPUT);
    pinMode(RAM_IO4, OUTPUT);
    pinMode(RAM_IO5, OUTPUT);
    pinMode(RAM_IO6, OUTPUT);
    pinMode(RAM_IO7, OUTPUT);
    pinMode(RAM_IO8, OUTPUT);
  } else {
    pinMode(RAM_IO1, INPUT);
    pinMode(RAM_IO2, INPUT);
    pinMode(RAM_IO3, INPUT);
    pinMode(RAM_IO4, INPUT);
    pinMode(RAM_IO5, INPUT);
    pinMode(RAM_IO6, INPUT);
    pinMode(RAM_IO7, INPUT);
    pinMode(RAM_IO8, INPUT);
  }
}

byte readData() {
  bool d7 = digitalRead(RAM_IO8);
  bool d6 = digitalRead(RAM_IO7);
  bool d5 = digitalRead(RAM_IO6);
  bool d4 = digitalRead(RAM_IO5);
  bool d3 = digitalRead(RAM_IO4);
  bool d2 = digitalRead(RAM_IO3);
  bool d1 = digitalRead(RAM_IO2);
  bool d0 = digitalRead(RAM_IO1);

  byte data_bus = d7<<7 | d6<<6 | d5<<5 | d4<<4 | d3<<3 | d2<<2 | d1<<1 | d0;

  return data_bus;
}

void writeData(byte b) {
  digitalWrite(RAM_IO1, bitRead(b, 0));
  digitalWrite(RAM_IO2, bitRead(b, 1));
  digitalWrite(RAM_IO3, bitRead(b, 2));
  digitalWrite(RAM_IO4, bitRead(b, 3));
  digitalWrite(RAM_IO5, bitRead(b, 4));
  digitalWrite(RAM_IO6, bitRead(b, 5));
  digitalWrite(RAM_IO7, bitRead(b, 6));
  digitalWrite(RAM_IO8, bitRead(b, 7));
}

void writeAddress(byte b) {
  digitalWrite(RAM_A0, bitRead(b, 0));
  digitalWrite(RAM_A1, bitRead(b, 1));
  digitalWrite(RAM_A2, bitRead(b, 2));
  digitalWrite(RAM_A3, bitRead(b, 3));
  digitalWrite(RAM_A4, bitRead(b, 4));
  digitalWrite(RAM_A5, bitRead(b, 5));
}

void writeCycle(byte data, word address) {
  // Memorizza la parte alta dell'indirizzo nel 374
  byte high_add = (address >> 6);
  writeAddress(high_add);
  digitalWrite(RAM_SELECT_HIGH_ADD, 0);
  delayMicroseconds(3);
  digitalWrite(RAM_SELECT_HIGH_ADD, 1);
  delayMicroseconds(3);
  /////////

  writeAddress(address & 0x3F);
  delayMicroseconds(3);
  digitalWrite(RAM_OE, 1);
  delayMicroseconds(3);
  digitalWrite(RAM_CS, 0);
  delayMicroseconds(3);
  setDatabusOut(true);
  writeData(data);
  delayMicroseconds(3);
  digitalWrite(RAM_WE, 0);
  delayMicroseconds(3);
  digitalWrite(RAM_WE, 1);
  delayMicroseconds(3);
  setDatabusOut(false);
  delayMicroseconds(3);
  digitalWrite(RAM_CS, 1);
  delayMicroseconds(3);
}

byte readCycle(word address) {
  // Memorizza la parte alta dell'indirizzo nel 374
  byte high_add = (address >> 6);
  writeAddress(high_add);
  digitalWrite(RAM_SELECT_HIGH_ADD, 0);
  delayMicroseconds(3);
  digitalWrite(RAM_SELECT_HIGH_ADD, 1);
  delayMicroseconds(3);
  /////////

  writeAddress(address & 0x3F);
  digitalWrite(RAM_CS, 0);
  delayMicroseconds(3);
  digitalWrite(RAM_OE, 0);
  delayMicroseconds(3);
  byte data = readData();
  digitalWrite(RAM_OE, 1);
  delayMicroseconds(3);
  digitalWrite(RAM_CS, 1);
  delayMicroseconds(3);

  return data;
}

byte mem_content[1024];
const int MEM_ARRAY_SIZE = 1024;
const int PHISICAL_MEMORY_SIZE = 2048;

void readCheckIteration() {
  char buffer[400];

  int tot_errors = 0;

  byte data_read;

  for (int i=0; i<MEM_ARRAY_SIZE; ++i) {
    data_read = readCycle(i);
    if (data_read != mem_content[i]) {
      sprintf(buffer, "Errore all'indirizzo 0x%04X - Valore letto: 0x%02X  Valore scritto: 0x%02X", i, data_read, mem_content[i]);
      Serial.println(buffer);

      ++tot_errors;
    }

    data_read = readCycle(i + MEM_ARRAY_SIZE);
    if (data_read != mem_content[i]) {
      sprintf(buffer, "Errore all'indirizzo 0x%04X - Valore letto: 0x%02X  Valore scritto: 0x%02X", i, data_read, mem_content[i]);
      Serial.println(buffer);

      ++tot_errors;
    }
  }
  Serial.print("Totali errori: ");
  Serial.println(tot_errors);

  Serial.println("");
}

void setup() {
  Serial.begin(115200);

  setDatabusOut(false);

  pinMode(RAM_A0, OUTPUT);
  pinMode(RAM_A1, OUTPUT);
  pinMode(RAM_A2, OUTPUT);
  pinMode(RAM_A3, OUTPUT);
  pinMode(RAM_A4, OUTPUT);
  pinMode(RAM_A5, OUTPUT);
  
  pinMode(RAM_WE, OUTPUT);
  pinMode(RAM_OE, OUTPUT);
  pinMode(RAM_CS, OUTPUT);

  pinMode(RAM_SELECT_HIGH_ADD, OUTPUT);

  digitalWrite(RAM_WE, 1);
  digitalWrite(RAM_OE, 1);
  digitalWrite(RAM_CS, 1);

  digitalWrite(RAM_SELECT_HIGH_ADD, 1);

  randomSeed(321);

  // Inizializza l'array con i valori da scrivere in memoria con numeri casuali
  // L'ATmega328 non ha abbastanza memoria quindi usiamo un array di dimensioni pari
  // alla metà della dimensione della RAM e si scrive lo stesso valore due volte
  for (int i=0; i<MEM_ARRAY_SIZE; ++i) {
    byte rand_data = random(0, 255);
    mem_content[i] = rand_data;
  }

  // Effettua il ciclo di scrittura
  Serial.println("Inizio scrittura");
  for (int i=0; i<MEM_ARRAY_SIZE; ++i) {
    byte rand_data = random(0, 255);

    mem_content[i] = rand_data;
    writeCycle(rand_data, i);
    writeCycle(rand_data, i + MEM_ARRAY_SIZE);

    if (i%10 == 0) {
      Serial.print(".");
    }
  }
  Serial.println("");
  Serial.println("Fine scrittura");
  Serial.println("");

  delay(10);

  Serial.println("Prima iterazione");

  readCheckIteration();
  
  delay(10 * 1000UL);

  Serial.println("Seconda iterazione (dopo 10 s)");
  readCheckIteration(); 

  delay(10 * 60 * 1000UL);

  Serial.println("Terza iterazione (dopo 10 minuti");
  readCheckIteration(); 
}

void loop() {
}

