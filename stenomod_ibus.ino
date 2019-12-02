// The idea is to get a stenomod (which outputs tx bolt)
// to output IBUS instead, which vJoySerialFeeder can
// pick up and turn into a game controller.

#include "ibus.h"
#include <limits.h>
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

const byte ROWS = 6;
const byte COLS = 4;

// Right side keys are all lowercase, S2 is X
const char keys[ROWS][COLS] = {
    {'H', 'u', 'g', 'S'},
    {'W', 'e', 'l', '#'},
    {'P', '*', 'b', 'z'},
    {'K', 'O', 'p', 'd'},
    {'T', 'A', 'r', 's'},
    {'X', 'R', 'f', 't'},
};

const byte rowPins[ROWS] = {19, 18, 17, 16, 15, 14};
const byte colPins[COLS] = {11, 10, 9, 8}; 

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
IBus ibus(NUM_CHANNELS);

void setup() {
    analogReference(ANALOG_REFERENCE); // use the defined ADC reference voltage source
    Serial.begin(BAUD_RATE);           // setup serial
}

void loop() {
    int bit, row, col, bm_ch = 0;
    unsigned long time = millis();
    
    ibus.begin();

    kpd.getKeys();
    //scan the entire keymap, querying individually
    for(col = 0; col < COLS; col++) {
        for(row = 0; row < ROWS; row++) {
            bool keyPressed = false;

            //search the list of keys for the one we're working on right now
            for (byte i = 0; i < LIST_MAX; i++) {
                 if (kpd.key[i].kchar == keys[row][col]) {
                     if ((kpd.key[i].kstate == PRESSED) || (kpd.key[i].kstate == HOLD)) {
                        keyPressed = true;
                     }
                 }
            }
            if(keyPressed) {
                bm_ch |= 1 << bit;
            }
            if(bit == 15 || (row == ROWS - 1 && col == COLS - 1)) { // data for one channel ready
                ibus.write(bm_ch);
                bm_ch = 0;
                bit = 0;
                continue;
            }
            bit++;
        }
    }

/*
    for (int i = 0; i < NUM_DIGITAL_BITMAPPED_INPUTS; i++) { // Scan the whole key list.
        int bit = i % 16;
        if (kpd.key[i].kstate == PRESSED) { // Find if it's being held down.
            bm_ch |= 1 << bit; // set a bit for each depressed key
        }
        if(bit == 15 || i == NUM_DIGITAL_BITMAPPED_INPUTS - 1) { // data for one channel ready
            ibus.write(bm_ch);
            bm_ch = 0;
        }
    }
*/
    ibus.end();
    
    time = millis() - time; // time elapsed in reading the inputs
    if(time < UPDATE_INTERVAL) // sleep till it is time for the next update
        delay(UPDATE_INTERVAL  - time);
}
