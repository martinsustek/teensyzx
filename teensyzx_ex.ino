const int ledPin = 13;

#include "/home/martin/bastleni/teensy/teensyzx/DrZ80/drz80.h"
#include "/home/martin/bastleni/teensy/teensyzx/DrZ80/drz80_processed.s"

struct DrZ80 MyCpu;

//uint8_t Z80_Ram[48 * 1024];
uint8_t Z80_Ram[16 * 1024];
uint8_t Z80_Rom[16 * 1024];

unsigned int z80_rebaseSP(unsigned short new_sp) {
  if (new_sp > 0xC000) { // PC in memory - rebase PC into memory
    MyCpu.Z80SP_BASE = (unsigned int)&Z80_Ram - 0xC000;
    return (MyCpu.Z80SP_BASE + new_sp);
  } else { // PC in rom - rebase PC into rom
    MyCpu.Z80SP_BASE = (unsigned int)&Z80_Rom;
    return (MyCpu.Z80SP_BASE + new_sp);
  }
}

unsigned int z80_rebasePC(unsigned short new_pc) {
  if (new_pc > 0xC000) { // PC in memory - rebase PC into memory
    MyCpu.Z80PC_BASE = (unsigned int)&Z80_Ram - 0xC000;
    return (MyCpu.Z80PC_BASE + new_pc);
  } else { // PC in rom - rebase PC into rom
    MyCpu.Z80PC_BASE = (unsigned int)&Z80_Rom;
    return (MyCpu.Z80PC_BASE + new_pc);
  }
}

unsigned char z80_read8(unsigned short a) {
}
unsigned short z80_read16(unsigned short a) {
}
void z80_write8(unsigned char d, unsigned short a) {
}
void z80_write16(unsigned short d, unsigned short a) {
}
unsigned char z80_in(short unsigned int p) {
}
void z80_out(short unsigned int p, unsigned char d) {
}

void z80_init() {
  memset(&MyCpu, 0, sizeof(MyCpu));
  
  // save pointers to all required memory functions
  MyCpu.z80_rebasePC = z80_rebasePC;
  MyCpu.z80_rebaseSP = z80_rebaseSP;
  MyCpu.z80_read8 = z80_read8;
  MyCpu.z80_read16 = z80_read16;
  MyCpu.z80_write8 = z80_write8;
  MyCpu.z80_write16 = z80_write16;
  MyCpu.z80_in = z80_in;
  MyCpu.z80_out = z80_out;
  MyCpu.z80_irq_callback=z80_irq_callback;

}

void z80_reset() {
  MyCpu.Z80A = 0x00     <<24;
  MyCpu.Z80F = (1<<2)   <<24;  // set ZFlag
  MyCpu.Z80BC = 0x0000  <<16;
  MyCpu.Z80DE = 0x0000  <<16;
  MyCpu.Z80HL = 0x0000  <<16;
  MyCpu.Z80A2 = 0x00    <<24;
  MyCpu.Z80F2 = 1<<2    <<24;  // set ZFlag
  MyCpu.Z80BC2 = 0x0000 <<16;
  MyCpu.Z80DE2 = 0x0000 <<16;
  MyCpu.Z80HL2 = 0x0000 <<16;
  MyCpu.Z80IX = 0xFFFF  <<16;
  MyCpu.Z80IY = 0xFFFF  <<16;
  
  MyCpu.Z80I = 0x00;
  MyCpu.Z80IM = 0x00;
  MyCpu.Z80_IRQ = 0x00;
  MyCpu.Z80IF = 0x00;
  
  MyCpu.Z80PC = z80_rebasePC(0);
  MyCpu.Z80SP = z80_rebaseSP(0);
}

void z80_set_irq(unsigned int irq) {
  MyCpu.z80irqvector = 0xFF;  // default IRQ vector RST opcode
  MyCpu.Z80_IRQ = irq;
}

void z80_irq_callback(void)
{
  MyCpu.Z80_IRQ = 0x00;  // lower irq when in accepted
}







void setup() {
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);
  
  Serial.begin(9600);
  
  z80_init();
  z80_reset();
}

void loop() {
  digitalWriteFast(ledPin, HIGH);
  delay(50);
  digitalWriteFast(ledPin, LOW);
  delay(950);
  
  DrZ80Run(&MyCpu, 100);
  
  Serial.write("*");
}
