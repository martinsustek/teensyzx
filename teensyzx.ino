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
  cycles_emulated = 0;

  int ram_checksum = 0;
  int ram_nonzero = 0;
  for (int i = 0; i < 48 * 1024; i++) {
    ram_checksum += RAM[i];
    if (RAM[i] != 0) {
      ram_nonzero++;
    }
  }
  Serial.printf("PC: %x, DE: %x, HL: %x, RAM checksum: %d, non-zero bytes: %d\n", state.pc, state.registers.word[Z80_DE], state.registers.word[Z80_HL], ram_checksum, ram_nonzero);

  for (int y = 0; y < 192; y++) {
    for (int x = 0; x < 32; x++) {
      int r = ((y % 8) << 3) | (((y / 8) % 8)) | ((y / 64) << 6);
      int i = r * 32 + x;
      Serial.print((RAM[i] & 0b10000000) ? "X" : " ");
      Serial.print((RAM[i] & 0b01000000) ? "X" : " ");
      Serial.print((RAM[i] & 0b00100000) ? "X" : " ");
      Serial.print((RAM[i] & 0b00010000) ? "X" : " ");
      Serial.print((RAM[i] & 0b00001000) ? "X" : " ");
      Serial.print((RAM[i] & 0b00000100) ? "X" : " ");
      Serial.print((RAM[i] & 0b00000010) ? "X" : " ");
      Serial.print((RAM[i] & 0b00000001) ? "X" : " ");
    }
    Serial.printf("\n");
  }
}

#ifdef __cplusplus
}
#endif
