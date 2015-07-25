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

void display_init() {
  myDisplay = UTFT(ILI9325D_8, DISPLAY_RS, DISPLAY_WR, DISPLAY_CS, DISPLAY_RST);
  myDisplay.InitLCD();
  myDisplay.clrScr();
}

uint8_t display_draw_start_line = 0;

inline void display_draw() {
  //cbi(myDisplay.P_CS, myDisplay.B_CS);
  digitalWriteFast(DISPLAY_CS, LOW);
  for (int line_number = display_draw_start_line; line_number < 192; line_number += 2) {
    myDisplay.setXY(32, 24 + line_number, 32+256-1, 24 + line_number);
    //sbi(myDisplay.P_RS, myDisplay.B_RS);
    digitalWriteFast(DISPLAY_RS, HIGH);
  
    uint16_t line_start = (((line_number % 8) << 3) | (((line_number / 8) % 8)) | ((line_number / 64) << 6)) * 32;
    for (int x = 31; x >= 0; x--) {
      //uint16_t fg_color = 0x0000;
      //uint16_t bg_color = 0xFFFF;
      uint8_t mini_line = RAM[line_start + x];
      uint8_t mask = 0b00000001;
      for (uint8_t i = 0; i < 8; i++) {
        if (mini_line & mask) {
          *(volatile uint8_t *)(&GPIOD_PDOR) = 0x00;
          //pulse_low(myDisplay.P_WR, myDisplay.B_WR);
          digitalWriteFast(DISPLAY_WR, LOW);
          __asm__("nop");
          digitalWriteFast(DISPLAY_WR, HIGH);
          *(volatile uint8_t *)(&GPIOD_PDOR) = 0x00;
          //pulse_low(myDisplay.P_WR, myDisplay.B_WR);
          digitalWriteFast(DISPLAY_WR, LOW);
          __asm__("nop");
          digitalWriteFast(DISPLAY_WR, HIGH);
        } else {
          *(volatile uint8_t *)(&GPIOD_PDOR) = 0xFF;
          //pulse_low(myDisplay.P_WR, myDisplay.B_WR);
          digitalWriteFast(DISPLAY_WR, LOW);
          __asm__("nop");
          digitalWriteFast(DISPLAY_WR, HIGH);
          *(volatile uint8_t *)(&GPIOD_PDOR) = 0xFF;
          //pulse_low(myDisplay.P_WR, myDisplay.B_WR);
          digitalWriteFast(DISPLAY_WR, LOW);
          __asm__("nop");
          digitalWriteFast(DISPLAY_WR, HIGH);
        }
        mask = mask << 1;
      }
    }
  }
  //sbi(myDisplay.P_CS, myDisplay.B_CS);
  digitalWriteFast(DISPLAY_CS, HIGH);

  display_draw_start_line ^= 1;
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
  for (int i = 0; i < 50; i++) {
    millis0 = millis();
    cycles_emulated += Z80Emulate(&state, 70000);
    millis1 = millis();
    display_draw();
    millis2 = millis();
    Serial.printf("Emulation took %d ms, rendering took %d ms, total %d ms.\n", millis1 - millis0, millis2 - millis1, millis2 - millis0);
  }

  //Serial.printf("cycles emulated: %d\n", cycles_emulated);
  //cycles_emulated = 0;

}

#ifdef __cplusplus
}
#endif
