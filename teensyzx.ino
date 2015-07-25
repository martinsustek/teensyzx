#include <memorysaver.h>
#include <UTFT.h>

#ifdef __cplusplus
extern "C" {
#endif



#include "z80emu.h"

const int ledPin = 13;

#define DISPLAY_RS (23)
#define DISPLAY_WR (22)
#define DISPLAY_CS (3)
#define DISPLAY_RST (4)



int cycles_emulated;
UTFT myDisplay;
union {
  uint8_t b8[512];
  uint16_t b16[256];
} display_line_buffer;

int millis_A;
int millis_B;

void display_init() {
  myDisplay = UTFT(ILI9325D_8, DISPLAY_RS, DISPLAY_WR, DISPLAY_CS, DISPLAY_RST);
  myDisplay.InitLCD();
  myDisplay.clrScr();
}

inline void display_draw_bitmap_fast(int x, int y) {
  unsigned int col;
  int tx;

  int millis_0, millis_1, millis_2;

  myDisplay.setXY(x, y, x+256-1, y);
  sbi(myDisplay.P_RS, myDisplay.B_RS);
  for (tx=512-1; tx>=0; tx--) {
    *(volatile uint8_t *)(&GPIOD_PDOR) = display_line_buffer.b8[tx];
    pulse_low(myDisplay.P_WR, myDisplay.B_WR);
  } 
}

inline void display_draw_line(uint8_t line_number) {
  uint16_t line_start = (((line_number % 8) << 3) | (((line_number / 8) % 8)) | ((line_number / 64) << 6)) * 32;
  for (int x = 0; x < 32; x++) {
    uint16_t fg_color = 0x0000;
    uint16_t bg_color = 0xFFFF;
    uint8_t mask = 0b10000000;
    for (int i = 0; i < 8; i++) {
      if (RAM[line_start + x] & mask) {
        display_line_buffer.b16[x * 8 + i] = fg_color;
      } else {
        display_line_buffer.b16[x * 8 + i] = bg_color;
      }
      mask = mask >> 1;
    }
  }
  //myDisplay.drawBitmap((320 - 256) / 2, (240 - 192) / 2 + line_number, 256, 1, display_line_buffer);
  display_draw_bitmap_fast(32, 24 + line_number);
}

inline void display_draw() {
  cbi(myDisplay.P_CS, myDisplay.B_CS);
  for (int line_number = 0; line_number < 192; line_number++) {
    display_draw_line(line_number);
  }
  sbi(myDisplay.P_CS, myDisplay.B_CS);
}


void setup() {
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);
  
  Serial.begin(9600);
  
  Z80Reset(&state);  
  cycles_emulated = 0;

  display_init();
}


void loop() {
  int millis0, millis1, millis2;
  
  digitalWriteFast(ledPin, HIGH);
  delay(50);
  digitalWriteFast(ledPin, LOW);
  //delay(950);

  //Serial.printf("Emulating... ");
  for (int i = 0; i < 25; i++) {
    millis_A = 0;
    millis_B = 0;
    millis0 = millis();
    cycles_emulated += Z80Emulate(&state, 140000);
    millis1 = millis();
    display_draw();
    millis2 = millis();
    Serial.printf("Emulation took %d ms, rendering took %d ms (A %d, B %d), total %d ms.\n", millis1 - millis0, millis2 - millis1, millis_A, millis_B, millis2 - millis0);
  }

  //Serial.printf("cycles emulated: %d\n", cycles_emulated);
  //cycles_emulated = 0;

}

#ifdef __cplusplus
}
#endif
