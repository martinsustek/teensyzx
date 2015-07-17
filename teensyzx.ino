#ifdef __cplusplus
extern "C" {
#endif



#include "z80emu.h"

const int ledPin = 13;



int cycles_emulated;


void setup() {
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);
  
  Serial.begin(9600);
  
  Z80Reset(&state);  
  cycles_emulated = 0;
}

void loop() {
  digitalWriteFast(ledPin, HIGH);
  delay(50);
  digitalWriteFast(ledPin, LOW);
  //delay(950);

  Serial.printf("Emulating... ");
  for (int i = 0; i < 35; i++) {
    cycles_emulated += Z80Emulate(&state, 100000);
  }

  Serial.printf("cycles emulated: %d\n", cycles_emulated);

  int ram_checksum = 0;
  int ram_nonzero = 0;
  for (int i = 0; i < 48 * 1024; i++) {
    ram_checksum += RAM[i];
    if (RAM[i] != 0) {
      ram_nonzero++;
    }
  }
  Serial.printf("Registers: %d %d %d %d %d %d %d, RAM checksum: %d, non-zero bytes: %d\n", state.registers.word[0], state.registers.word[1], state.registers.word[2], state.registers.word[3], state.registers.word[4], state.registers.word[5], state.registers.word[6], ram_checksum, ram_nonzero);
}

#ifdef __cplusplus
}
#endif
