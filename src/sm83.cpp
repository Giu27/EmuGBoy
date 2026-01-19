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
        
        case 0x04:{//INC B
            bool h = (registers.b & 0x0F) == 0x0F;
            registers.b++;
            setFlag('n', false);
            setFlag('z', registers.b == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x05:{//DEC B
            bool h = (registers.b & 0x0F) == 0;
            registers.b--;
            setFlag('n', true);
            setFlag('z', registers.b == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x06: //LD B n
            registers.b = gb->readMemory(registers.pc);
            registers.pc++;
            cycles += 8;
            break;
        
        case 0x07: {//RLCA
            uint8_t b7 = getBit(registers.a, 7);
            registers.a = (registers.a << 1) | b7;
            setFlag('z', false);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', b7);
            cycles += 4;
            break;
        }

        case 0x08:{ //LD [a16] SP
            uint16_t nn = bytesToWord(gb->readMemory(registers.pc),gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            gb->writeMemory(nn, getLSB(registers.sp));
            gb->writeMemory(nn + 1, getMSB(registers.sp));
            cycles += 20;
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

        case 0x0A: //LD A [BC]
            registers.a = gb->readMemory(registers.bc);
            cycles += 8;
            break;
        
        case 0x0B: //DEC BC
            registers.bc--;
            cycles += 8;
            break;
        
        case 0x0C:{//INC C
            bool h = (registers.c & 0x0F) == 0x0F;
            registers.c++;
            setFlag('n', false);
            setFlag('z', registers.c == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x0D:{//DEC C
            bool h = (registers.c & 0x0F) == 0;
            registers.c--;
            setFlag('n', true);
            setFlag('z', registers.c == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x0E: //LD C n
            registers.c = gb->readMemory(registers.pc);
            registers.pc++;
            cycles += 8;
            break;
        
        case 0x0F:{//RRCA
            uint8_t b0 = getBit(registers.a, 0);
            registers.a = (registers.a >> 1) | (b0 << 7);
            setFlag('z', false);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', b0);
            cycles += 4;
            break;
        }

        case 0x11: //LD DE nn
            registers.de = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            cycles += 12;
            break;
        
        case 0x12: //LD [DE] A
            gb->writeMemory(registers.bc, registers.a);
            cycles += 8;
            break;

        case 0x13: //INC DE
            registers.de++;
            cycles += 8;
            break;

        case 0x20:{//JR CC e
            int8_t e = gb->readMemory(registers.pc);
            registers.pc++;
            if (!getFlag('z')) {
                registers.pc += e;
                cycles += 4;
            }
            cycles+=8;
            break;
        }

        case 0x21: //LD HL nn
            registers.hl = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            cycles += 12;
            break;

        case 0x2F: //CPL
            registers.a = ~registers.a;
            setFlag('n', true);
            setFlag('h', true);
            cycles += 4;
            break;

        case 0x31: //LD SP nn
            registers.sp = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            cycles += 12;
            break;
        
        case 0x32: //LD [HL-] A
            gb->writeMemory(registers.hl, registers.a);
            registers.hl--;
            cycles += 8;
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
        
        case 0xAF: //XOR A A
            registers.a = 0;
            setFlag('z', true);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;
        
        case 0xC3: //JP nn
            registers.pc = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            cycles += 16;
            break;
        
        case 0xCD:{//CALL nn
            uint16_t nn = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            registers.sp--;
            gb->writeMemory(registers.sp, getMSB(registers.pc));
            registers.sp--;
            gb->writeMemory(registers.sp, getLSB(registers.pc));
            registers.pc = nn;
            cycles += 24;
            break;
        }

        default:
            std::cout<<std::hex<<(int)registers.ir<<"\n";
            break;
    }

    return cycles;
}

bool Cpu::getFlag(char flag) {
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
    return getBit(registers.f, bit);
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