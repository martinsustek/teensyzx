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

#define KEYBOARD_11 (15)
#define KEYBOARD_12 (16)
#define KEYBOARD_13 (17)
#define KEYBOARD_14 (18)
#define KEYBOARD_15 (19)
#define KEYBOARD_21 (2) // SAME AS DISPLAY OUTPUTS
#define KEYBOARD_22 (14)
#define KEYBOARD_23 (7)
#define KEYBOARD_24 (8)
#define KEYBOARD_25 (6)
#define KEYBOARD_26 (20)
#define KEYBOARD_27 (21)
#define KEYBOARD_28 (5)


int cycles_emulated;
UTFT myDisplay;

uint8_t display_palette_high[] = {0b00000000, 0b00000000, 0b11010000, 0b11010000, 0b00000110, 0b00000110, 0b11010110, 0b11010110, 0b00000000, 0b00000000, 0b11111000, 0b11111000, 0b00000111, 0b00000111, 0b11111111, 0b11111111};
uint8_t display_palette_low[] =  {0b00000000, 0b00011010, 0b00000000, 0b00011010, 0b01100000, 0b01111010, 0b01100000, 0b01111010, 0b00000000, 0b00011111, 0b00000000, 0b00011111, 0b11100000, 0b11111111, 0b11100000, 0b11111111};

uint16_t display_line_number = 0;
uint8_t display_flashing = 0;
elapsedMicros display_timer;
elapsedMicros cpu_timer;


void display_init() {
  myDisplay = UTFT(ILI9325D_8, DISPLAY_RS, DISPLAY_WR, DISPLAY_CS, DISPLAY_RST);
  myDisplay.InitLCD();
  myDisplay.fillScr(205, 205, 205);
}

inline void display_draw_line(int line_number) {
  digitalWriteFast(DISPLAY_CS, LOW);
  myDisplay.setXY(32, 24 + line_number, 32+256-1, 24 + line_number);
  digitalWriteFast(DISPLAY_RS, HIGH);

  uint16_t line_start = (((line_number % 8) << 3) | (((line_number / 8) % 8)) | ((line_number / 64) << 6)) * 32;
  uint16_t attr_start = (0x1800 + ((line_number / 8) * 32));
  for (int x = 31; x >= 0; x--) {
    uint8_t attributes = RAM[attr_start + x];
    uint8_t fg_color_high = display_palette_high[(attributes & 0b00000111) | ((attributes & 0b01000000) >> 3)];
    uint8_t fg_color_low =  display_palette_low[(attributes & 0b00000111) | ((attributes & 0b01000000) >> 3)];
    uint8_t bg_color_high = display_palette_high[(attributes & 0b01111000) >> 3];
    uint8_t bg_color_low =  display_palette_low[(attributes & 0b01111000) >> 3];
    uint8_t flash = ((attributes & 0b1000000) && (display_flashing >= 16));
    uint8_t mini_line = RAM[line_start + x];
    uint8_t mask = 0b00000001;
    for (uint8_t i = 0; i < 8; i++) {
      if (((mini_line & mask) && !flash) || ((!(mini_line & mask)) && flash)) {
        *(volatile uint8_t *)(&GPIOD_PDOR) = fg_color_high;
        digitalWriteFast(DISPLAY_WR, LOW);
        __asm__("nop");
        digitalWriteFast(DISPLAY_WR, HIGH);
        *(volatile uint8_t *)(&GPIOD_PDOR) = fg_color_low;
        digitalWriteFast(DISPLAY_WR, LOW);
        __asm__("nop");
        digitalWriteFast(DISPLAY_WR, HIGH);
      } else {
        *(volatile uint8_t *)(&GPIOD_PDOR) = bg_color_high;
        digitalWriteFast(DISPLAY_WR, LOW);
        __asm__("nop");
        digitalWriteFast(DISPLAY_WR, HIGH);
        *(volatile uint8_t *)(&GPIOD_PDOR) = bg_color_low;
        digitalWriteFast(DISPLAY_WR, LOW);
        __asm__("nop");
        digitalWriteFast(DISPLAY_WR, HIGH);
      }
      mask = mask << 1;
    }
  }

  digitalWriteFast(DISPLAY_CS, HIGH);
}


inline void keyboard_init() {
  pinMode(KEYBOARD_11, INPUT_PULLUP);
  pinMode(KEYBOARD_12, INPUT_PULLUP);
  pinMode(KEYBOARD_13, INPUT_PULLUP);
  pinMode(KEYBOARD_14, INPUT_PULLUP);
  pinMode(KEYBOARD_15, INPUT_PULLUP);
}

inline uint8_t keyboard_read(uint8_t address) {
  uint8_t out = 0;
  digitalWriteFast(DISPLAY_CS, HIGH);

  digitalWriteFast(KEYBOARD_21, ((~address) & 0b00001000) ? LOW : HIGH);
  digitalWriteFast(KEYBOARD_22, ((~address) & 0b00000100) ? LOW : HIGH);
  digitalWriteFast(KEYBOARD_23, ((~address) & 0b00000010) ? LOW : HIGH);
  digitalWriteFast(KEYBOARD_24, ((~address) & 0b00010000) ? LOW : HIGH);
  digitalWriteFast(KEYBOARD_25, ((~address) & 0b00100000) ? LOW : HIGH);
  digitalWriteFast(KEYBOARD_26, ((~address) & 0b00000001) ? LOW : HIGH);
  digitalWriteFast(KEYBOARD_27, ((~address) & 0b01000000) ? LOW : HIGH);
  digitalWriteFast(KEYBOARD_28, ((~address) & 0b10000000) ? LOW : HIGH);

  if (digitalRead(KEYBOARD_11) != 0) {
    out |= 0b00000001;  
  }
  if (digitalRead(KEYBOARD_12) != 0) {
    out |= 0b00000010;  
  }
  if (digitalRead(KEYBOARD_13) != 0) {
    out |= 0b00000100;  
  }
  if (digitalRead(KEYBOARD_14) != 0) {
    out |= 0b00001000;  
  }
  if (digitalRead(KEYBOARD_15) != 0) {
    out |= 0b00010000;  
  }

  digitalWriteFast(DISPLAY_CS, LOW);
  return out;
}

uint8_t input_byte(uint16_t port) {
  uint8_t out = 0;

  if ((port & 0xFF) == 0xFE) {
    out |= keyboard_read(port >> 8);
  }

  return out;
}





void setup() {
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);
  
  Serial.begin(9600);
  
  Z80Reset(&state);  
  cycles_emulated = 0;

  keyboard_init();
  display_init();
}

int cycle_measure = 0;
int cycle_measure_frame_counter = 0;

void loop() {

  if (cpu_timer * 35 >= cycles_emulated * 10) {
    if ((cpu_timer >= 1000000) && (cycles_emulated >= 3500000)) {
      cpu_timer -= 1000000;
      cycles_emulated -= 3500000;
    }

    int cycles = Z80Emulate(&state, 112);
    //Serial.printf("Cycles emulated: %d\n", cycles);
    cycles_emulated += cycles;
    cycle_measure += cycles;
  }


  while (display_timer >= 64) {
    display_timer -= 64;
    
    display_line_number++;
    if (display_line_number > 625) {
      display_line_number = 1;
      
      display_flashing++;
      if (display_flashing >= 32) {
        display_flashing = 0;
      }

      int cycles = Z80Interrupt(&state, 0xff);
      cycles_emulated += cycles;
      cycle_measure += cycles;

      cycle_measure_frame_counter++;
      if (cycle_measure_frame_counter > 50) {
        Serial.printf("CPU emulation speed %f MHz\n", (float)(cycle_measure) / 1000000);
        cycle_measure = 0;
        cycle_measure_frame_counter = 0;
      }
    }
    
    if ((display_line_number >= 60) && (display_line_number < 60+192) && ((display_line_number % 2) == 0)) {
      display_draw_line(display_line_number - 60);
    }
    if ((display_line_number >= 372) && (display_line_number < 372+192) && ((display_line_number % 2) == 1)) {
      display_draw_line(display_line_number - 372);
    }
  }

}

#ifdef __cplusplus
}
#endif
