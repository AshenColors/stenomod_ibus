// The idea is to get a stenomod (which outputs tx bolt)
// to output IBUS instead, which vJoySerialFeeder can
// pick up and turn into a game controller.

#include "ibus.h"
#include <Keypad.h>

// How often to send data?
#define UPDATE_INTERVAL 10 // milliseconds

// How many bitmapped inputs (aka buttons)?
#define NUM_DIGITAL_BITMAPPED_INPUTS 24

// Define the appropriate analog reference source. See
// https://www.arduino.cc/reference/en/language/functions/analog-io/analogreference/
#define ANALOG_REFERENCE EXTERNAL

// Define the baud rate
#define BAUD_RATE 115200

// We've got 24 button states to manage, and each channel holds 16 bits, so 2
#define NUM_CHANNELS 2
// /////////////////

//tx bolt stuff
byte b[4];
const byte LED = 13;
const byte ROWS = 6;
const byte COLS = 4;
const byte rowPins[ROWS] = {19, 18, 17, 16, 15, 14};
const byte colPins[COLS] = {11, 10, 9, 8};
// Right side keys are all lowercase, S2 is X
const char keys[ROWS][COLS] = {
  {'H', 'u', 'g', 'S'},
  {'W', 'e', 'l', '#'},
  {'P', '*', 'b', 'z'},
  {'K', 'O', 'p', 'd'},
  {'T', 'A', 'r', 's'},
  {'X', 'R', 'f', 't'},
};

void setup() {
  DDRB = DDRC = DDRD = 0;
  PORTC = PORTD = 0xff;
  PORTB = 0xf0;
  led(false);
  /* //debugging
  Serial.begin(9600);
  while(true) {
    scan_keys();
    Serial.write(b[0]);
    Serial.write(b[1]);
    Serial.write(b[2]);
    Serial.write(b[3]);
    Serial.write(0xff);
    delay(1000);
  } */
  //mode switch to ibus if big red asterisk is held down
  scan_keys();
  if(b[1] & 0x08) {
    for(byte i = 0; i < 3; i++) {
      delay(250);
      led(true);
      delay(250);
      led(false);
    }
    ibus_mode();
  } else {
    tx_bolt_mode();
  }
}

void loop() {
  //needed for ide purposes
}

void ibus_mode() {
  Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
  IBus ibus(NUM_CHANNELS);
  int bit, row, col, bm_ch = 0;
  unsigned long time = millis();

  analogReference(ANALOG_REFERENCE); // use the defined ADC reference voltage source
  Serial.begin(BAUD_RATE);           // setup serial

  //main loop
  while (true) {
    ibus.begin();

    kpd.getKeys();
    //scan the entire keymap, querying individually
    for (col = 0; col < COLS; col++) {
      for (row = 0; row < ROWS; row++) {
        bool keyPressed = false;

        //search the list of keys for the one we're working on right now
        for (byte i = 0; i < LIST_MAX; i++) {
          if (kpd.key[i].kchar == keys[row][col]) {
            if ((kpd.key[i].kstate == PRESSED) || (kpd.key[i].kstate == HOLD)) {
              keyPressed = true;
            }
          }
        }
        if (keyPressed) {
          bm_ch |= 1 << bit;
        }
        if (bit == 15 || (row == ROWS - 1 && col == COLS - 1)) { // data for one channel ready
          ibus.write(bm_ch);
          bm_ch = 0;
          bit = 0;
          continue;
        }
        bit++;
      }
    }

    ibus.end();

    time = millis() - time; // time elapsed in reading the inputs
    if (time < UPDATE_INTERVAL) // sleep till it is time for the next update
      delay(UPDATE_INTERVAL  - time);
  }
}

void tx_bolt_mode() {
  Serial.begin(9600);
  while (true) {
    scan_keys();
    send_stroke();
  }
}

//everything below this is a helper function for tx bolt mode

void led(bool on) {
  digitalWrite(LED, on ? HIGH : LOW);
}

void set_output(byte p) {
  pinMode(colPins[p], OUTPUT);
  digitalWrite(colPins[p], LOW);
}

void set_input(byte p) {
  pinMode(colPins[p], INPUT);
}

byte read_byte() {
  return PINC ^ 0x3f;
}

void send_byte(byte b) {
  Serial.write(b);
}

byte read_column(byte p) {
  byte ret;
  set_output(p);
  delayMicroseconds(10);
  ret = read_byte();
  set_input(p);
  return ret;
}

// Check all keys, and modify
// b array for new presses
// Return true if any key is pressed
bool look() {
  bool ret = false;
  for (int i = 0; i < 4; i++) {
    byte r = read_column(i);
    //figure out is S2 is pushed
    //that creates 11100000 when
    //it should just be 0000001
    if (i == 3 && r & 0x20) {
      ret = true;
      r &= ~0x20;
      b[0] |= 0x01;
    }
    ret |= r;
    b[i] |= r;
  }
  return ret;
}

// Continuously poll for keys until
// Keys are pressed and then released
void scan_keys() {
  byte key_pressed = false;
  b[0] = b[1] = b[2] = b[3] = 0;

  while (key_pressed == false) {
    while (look() == false);
    delay(20);
    key_pressed = look();
  }
  led(true);
  while (look() == true);
  led(false);
}

// Send the current stroke stored in
// b array
void send_stroke() {
  if (b[0])
    send_byte(b[0]);
  if (b[1])
    send_byte(b[1] | 0x40);
  if (b[2])
    send_byte(b[2] | 0x80);
  if (b[3])
    send_byte(b[3] | 0xc0);
  else
    send_byte(0);
}
