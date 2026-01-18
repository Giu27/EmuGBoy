//Copyright (C) 2026  Giuseppe Caruso
#include <iostream>
#include <gb.h>
#include <utils.h>
#include <sm83.h>

Cpu::Cpu(Gb* parent) : gb(parent){
    registers.pc = 0x0100; 
}

int Cpu::step() { //Returns number of T-cycles (M-Cycles = T-Cycles / 4)
    int cycles = 0;
    registers.ir = gb->readMemory(registers.pc);
    registers.pc++;

    switch (registers.ir) {
        case 0x00: //NOP
            cycles += 4;
            break;
        
        case 0x01: //LD BC nn
            registers.bc = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            cycles += 12;
            break;
        
        case 0x02: //LD [BC] A
            gb->writeMemory(registers.bc, registers.a);
            cycles += 8;
            break;
        
        case 0x03: //INC BC
            registers.bc++;
            cycles += 8;
            break;
        
        case 0x04: {//INC B
            bool h = (registers.b & 0x0F) == 0x0F;
            registers.b++;
            setFlag('n', false);
            setFlag('z', registers.b == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }
        
        case 0x09: {//ADD HL BC
            uint32_t result = registers.hl + registers.bc;
            bool h = ((registers.hl & 0xFFF) + (registers.bc & 0xFFF)) > 0xFFF;
            registers.hl = (uint16_t) (result & 0xFFFF);
            setFlag('n', false);
            setFlag('h', h);
            setFlag('c', result > 0xFFFF);
            cycles += 8;
            break;
        }

        case 0x2F: //CPL
            registers.a = ~registers.a;
            setFlag('n', true);
            setFlag('h', true);
            cycles += 4;
            break;

        case 0x31: //LD SP n16
            registers.sp = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            cycles += 12;
            break;

        case 0x40: //LD B r8
            registers.b = registers.b;
            cycles += 4;
            break;
        case 0x41:
            registers.b = registers.c;
            cycles += 4;
            break;
        case 0x42:
            registers.b = registers.d;
            cycles += 4;
            break;
        case 0x43:
            registers.b = registers.e;
            cycles += 4;
            break;
        case 0x44:
            registers.b = registers.h;
            cycles += 4;
            break;
        case 0x45:
            registers.b = registers.l;
            cycles += 4;
            break;
        case 0x46: //LD B [HL] Loads data from address in HL
            registers.b = gb->readMemory(registers.hl);
            cycles += 8;
            break;
        case 0x47: //LD B A
            registers.b = registers.a;
            cycles += 4;
            break;
        
        case 0x60: //LD H, r8
            registers.h = registers.b;
            cycles += 4;
            break;
        
        case 0xC3: //JP nn
            registers.pc = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            cycles += 16;
            break;
    
        default:
            std::cout<<std::hex<<(int)registers.ir<<"\n";
            break;
    }

    return cycles;
}

void Cpu::setFlag(char flag, bool val) {
    int bit;
    switch (flag) {
        case 'z':
            bit = 7;
            break;
        case 'n':
            bit = 6;
            break;
        case 'h':
            bit = 5;
            break;
        case 'c':
            bit = 4;
            break;
    }
    if (val) {
        setBit(registers.f, bit);
    } else {
        clearBit(registers.f, bit);
    }
}