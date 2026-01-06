#include <sm83.h>

Cpu::Cpu(){
    registers.pc = 0x0100; //For now those are test values to be ignored
    registers.bc = 0xFFFA;
    registers.d = 0xFF;
    registers.l = 0xAA;
}