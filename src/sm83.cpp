#include <sm83.h>

#include <utils.h>

Cpu::Cpu(){
    registers.pc = 0x0100; //For now those are all test values to be ignored
    registers.bc = 0xFFFA;
    registers.d = 0xFF;
    registers.l = 0xAA;
    registers.f = 0b01010000;

    setBit(registers.f, 7);
    setBit(registers.f, 5);
    clearBit(registers.f, 6);
    clearBit(registers.f, 4);
}