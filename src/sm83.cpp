//Copyright (C) 2026  Giuseppe Caruso
#include <iostream>
#include <gb.h>
#include <utils.h>
#include <sm83.h>

//FILE* log_file = fopen("cpuLog.txt", "w");

Cpu::Cpu(Gb* parent) : gb(parent),timer(this){ //Initial Values
    registers.pc = 0x0100; 
    registers.sp = 0xFFFE;
    registers.l = 0x4D;
    registers.h = 0x01;
    registers.e = 0xD8;
    registers.d = 0x00;
    registers.c = 0x13;
    registers.b = 0x00;
    registers.a = 0x01;
    setFlag('z', true);
    IME = false;
    HALT = false;
    gb->memory[0xFF0F] = 0xE1; //Initialize interrupts flags
}

int Cpu::step() { //Returns number of T-cycles (M-Cycles = T-Cycles / 4)
    //fprintf(log_file, "A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n", registers.a, registers.f, registers.b, registers.c, registers.d, registers.e, registers.h, registers.l, registers.sp, registers.pc, gb->memory[registers.pc],gb->memory[registers.pc+1],gb->memory[registers.pc+2],gb->memory[registers.pc+3]);
    int cycles = 0;

    handleInterrupts();

    if (!HALT) {
        registers.ir = gb->readMemory(registers.pc);
        registers.pc++;
    }
    

    switch (registers.ir) {
        case 0xCB:{//PREFIX
            registers.ir = gb->readMemory(registers.pc);
            registers.pc++;
            switch (registers.ir) {
                case 0x19:{//RR C
                    uint8_t b0 = getBit(registers.c, 0);
                    registers.c = (registers.c >> 1) | (getFlag('c') << 7);
                    setFlag('z', registers.c == 0);
                    setFlag('n', false);
                    setFlag('h', false);
                    setFlag('c', b0);
                    cycles += 8;
                    break;
                }
                case 0x1A:{//RR D
                    uint8_t b0 = getBit(registers.d, 0);
                    registers.d = (registers.d >> 1) | (getFlag('c') << 7);
                    setFlag('z', registers.d == 0);
                    setFlag('n', false);
                    setFlag('h', false);
                    setFlag('c', b0);
                    cycles += 8;
                    break;
                }
                case 0x1B:{//RR E
                    uint8_t b0 = getBit(registers.e, 0);
                    registers.e = (registers.e >> 1) | (getFlag('c') << 7);
                    setFlag('z', registers.e == 0);
                    setFlag('n', false);
                    setFlag('h', false);
                    setFlag('c', b0);
                    cycles += 8;
                    break;
                }

                case 0x38:{//SRL B
                    uint8_t b0 = getBit(registers.b, 0);
                    registers.b = (registers.b >> 1) | (0 << 7);
                    setFlag('z', registers.b == 0);
                    setFlag('n', false);
                    setFlag('h', false);
                    setFlag('c', b0);
                    cycles += 8;
                    break;
                }
                default:
                    std::cout<<"CB "<<std::hex<<(int)registers.ir<<"\n";
                    break;
            }
            break;
        }

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
            gb->writeMemory(registers.de, registers.a);
            cycles += 8;
            break;

        case 0x13: //INC DE
            registers.de++;
            cycles += 8;
            break;

        case 0x14:{//INC D
            bool h = (registers.d & 0x0F) == 0x0F;
            registers.d++;
            setFlag('n', false);
            setFlag('z', registers.d == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x15:{//DEC D
            bool h = (registers.d & 0x0F) == 0;
            registers.d--;
            setFlag('n', true);
            setFlag('z', registers.d == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x16: //LD D n
            registers.d = gb->readMemory(registers.pc);
            registers.pc++;
            cycles += 8;
            break;

        case 0x18:{//JR e8
            int8_t e = (int8_t) gb->readMemory(registers.pc);
            registers.pc++;
            registers.pc += e;
            cycles += 12;
            break;
        }

        case 0x1A: //LD A [DE]
            registers.a = gb->readMemory(registers.de);
            cycles += 8;
            break;

        case 0x1B: //DEC DE
            registers.de--;
            cycles += 8;
            break;

        case 0x1C:{//INC E
            bool h = (registers.e & 0x0F) == 0x0F;
            registers.e++;
            setFlag('n', false);
            setFlag('z', registers.e == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x1D:{//DEC E
            bool h = (registers.e & 0x0F) == 0;
            registers.e--;
            setFlag('n', true);
            setFlag('z', registers.e == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x1E: //LD E n
            registers.e = gb->readMemory(registers.pc);
            registers.pc++;
            cycles += 8;
            break;

        case 0x1F:{//RRA
            uint8_t b0 = getBit(registers.a, 0);
            registers.a = (registers.a >> 1) | (getFlag('c') << 7);
            setFlag('z', false);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', b0);
            cycles += 4;
            break;
        }

        case 0x20:{//JR NZ e
            int8_t e = (int8_t) gb->readMemory(registers.pc);
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
        
        case 0x22: //LD [HL+] A
            gb->writeMemory(registers.hl, registers.a);
            registers.hl++;
            cycles += 8;
            break;
        
        case 0x23: //INC HL
            registers.hl++;
            cycles += 8;
            break;
        
        case 0x24:{//INC H
            bool h = (registers.h & 0x0F) == 0x0F;
            registers.h++;
            setFlag('n', false);
            setFlag('z', registers.h == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x25:{//DEC H
            bool h = (registers.h & 0x0F) == 0;
            registers.h--;
            setFlag('n', true);
            setFlag('z', registers.h == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x26: //LD H n
            registers.h = gb->readMemory(registers.pc);
            registers.pc++;
            cycles += 8;
            break;
        
        case 0x27:{//DAA
            uint8_t adjustment = 0;
            if (getFlag('h') || (!getFlag('n') && (registers.a & 0x0f)> 0x09)) {
                adjustment |= 0x06;
            }
            if (getFlag('c') || (!getFlag('n') && registers.a > 0x99)){
                adjustment |= 0x60;
                setFlag('c', true);
            }
            if (getFlag('n')) {
                registers.a -= adjustment;
            } else {
                registers.a += adjustment;
            }
            setFlag('z', registers.a == 0);
            setFlag('h', false);
            cycles += 4;
            break;
        }

        case 0x28:{//JR Z e
            int8_t e = (int8_t) gb->readMemory(registers.pc);
            registers.pc++;
            if (getFlag('z')) {
                registers.pc += e;
                cycles += 4;
            }
            cycles+=8;
            break;
        }

        case 0x29:{//ADD HL HL 
            uint32_t result = registers.hl + registers.hl;
            bool h = ((registers.hl & 0xFFF) + (registers.hl & 0xFFF)) > 0xFFF;
            registers.hl = (uint16_t) (result & 0xFFFF);
            setFlag('n', false);
            setFlag('h', h);
            setFlag('c', result > 0xFFFF);
            cycles += 8;
            break;
        }
        
        case 0x2A: //LD A [HL+]
            registers.a = gb->readMemory(registers.hl);
            registers.hl++;
            cycles += 8;
            break;

        case 0x2B: //DEC HL
            registers.hl--;
            cycles += 8;
            break;
        
        case 0x2C:{//INC L
            bool h = (registers.l & 0x0F) == 0x0F;
            registers.l++;
            setFlag('n', false);
            setFlag('z', registers.l == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x2D:{//DEC L
            bool h = (registers.l & 0x0F) == 0;
            registers.l--;
            setFlag('n', true);
            setFlag('z', registers.l == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x2E: //LD L n
            registers.l = gb->readMemory(registers.pc);
            registers.pc++;
            cycles += 8;
            break;

        case 0x2F: //CPL
            registers.a = ~registers.a;
            setFlag('n', true);
            setFlag('h', true);
            cycles += 4;
            break;

        case 0x30:{//JR NC e
            int8_t e = (int8_t) gb->readMemory(registers.pc);
            registers.pc++;
            if (!getFlag('c')) {
                registers.pc += e;
                cycles += 4;
            }
            cycles+=8;
            break;
        }

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
        
        case 0x33: //INC SP
            registers.sp++;
            cycles += 8;
            break;
        
        case 0x35:{//DEC [HL]
            uint8_t data = gb->readMemory(registers.hl);
            bool h = (data & 0x0F) == 0;
            data--;
            gb->writeMemory(registers.hl, data);
            setFlag('n', true);
            setFlag('z', data == 0);
            setFlag('h', h);
            cycles += 12;
            break;
        } 

        case 0x36: //LD [HL] n
            gb->writeMemory(registers.hl, gb->readMemory(registers.pc));
            registers.pc++;
            cycles += 12;
            break;

        case 0x38:{//JR C e
            int8_t e = (int8_t) gb->readMemory(registers.pc);
            registers.pc++;
            if (getFlag('c')) {
                registers.pc += e;
                cycles += 4;
            }
            cycles+=8;
            break;
        }

        case 0x39: {//ADD HL SP
            uint32_t result = registers.hl + registers.sp;
            bool h = ((registers.hl & 0xFFF) + (registers.sp & 0xFFF)) > 0xFFF;
            registers.hl = (uint16_t) (result & 0xFFFF);
            setFlag('n', false);
            setFlag('h', h);
            setFlag('c', result > 0xFFFF);
            cycles += 8;
            break;
        }

        case 0x3B: //DEC SP
            registers.sp--;
            cycles += 8;
            break;
        
        case 0x3C:{//INC A
            bool h = (registers.a & 0x0F) == 0x0F;
            registers.a++;
            setFlag('n', false);
            setFlag('z', registers.a == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x3D:{//DEC A
            bool h = (registers.a & 0x0F) == 0;
            registers.a--;
            setFlag('n', true);
            setFlag('z', registers.a == 0);
            setFlag('h', h);
            cycles += 4;
            break;
        }

        case 0x3E: //LD A n
            registers.a = gb->readMemory(registers.pc);
            registers.pc++;
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
        
        case 0x4E: //LD C [HL]
            registers.c = gb->readMemory(registers.hl);
            cycles += 8;
            break;
        case 0x4F: //LD C A
            registers.c = registers.a;
            cycles += 4;
            break;

        case 0x56: //LD D [HL]
            registers.d = gb->readMemory(registers.hl);
            cycles += 8;
            break;
        case 0x57: //LD D A
            registers.d = registers.a;
            cycles += 4;
            break;

        case 0x58: //LD E, r8
            registers.e = registers.b;
            cycles += 4;
            break;
        case 0x59: 
            registers.e = registers.c;
            cycles += 4;
            break;
        case 0x5A: 
            registers.e = registers.d;
            cycles += 4;
            break;
        case 0x5B: 
            registers.e = registers.e;
            cycles += 4;
            break;
        case 0x5C: 
            registers.e = registers.h;
            cycles += 4;
            break;
        case 0x5D: 
            registers.e = registers.l;
            cycles += 4;
            break;
        case 0x5E: //LD E [HL]
            registers.e = gb->readMemory(registers.hl);
            cycles += 8;
            break;
        case 0x5F: //LD E A
            registers.e = registers.a;
            cycles += 4;
            break;

        case 0x60: //LD H, r8
            registers.h = registers.b;
            cycles += 4;
            break;
        case 0x61: 
            registers.h = registers.c;
            cycles += 4;
            break;
        case 0x62: 
            registers.h = registers.d;
            cycles += 4;
            break;
        case 0x63: 
            registers.h = registers.e;
            cycles += 4;
            break;
        case 0x64: 
            registers.h = registers.h;
            cycles += 4;
            break;
        case 0x65: 
            registers.h = registers.l;
            cycles += 4;
            break;
        case 0x66: //LD H [HL]
            registers.h = gb->readMemory(registers.hl);
            cycles += 8;
            break;
        case 0x67: //LD H A
            registers.h = registers.a;
            cycles += 4;
            break;

        case 0x68: //LD L r8
            registers.l = registers.b;
            cycles += 4;
            break;
        case 0x69: 
            registers.l = registers.c;
            cycles += 4;
            break;
        case 0x6A: 
            registers.l = registers.d;
            cycles += 4;
            break;
        case 0x6B: 
            registers.l = registers.e;
            cycles += 4;
            break;
        case 0x6C: 
            registers.l = registers.h;
            cycles += 4;
            break;
        case 0x6D: 
            registers.l = registers.l;
            cycles += 4;
            break;        
        case 0x6E: //LD L [HL]
            registers.l = gb->readMemory(registers.hl);
            cycles += 8;
            break;        
        case 0x6F: //LD L A
            registers.l = registers.a;
            cycles += 4;
            break;

        case 0x70: //LD [HL] r8
            gb->writeMemory(registers.hl, registers.b);
            cycles += 8;
            break;
        case 0x71:
            gb->writeMemory(registers.hl, registers.c);
            cycles += 8;
            break;
        case 0x72:
            gb->writeMemory(registers.hl, registers.d);
            cycles += 8;
            break;
        case 0x73:
            gb->writeMemory(registers.hl, registers.e);
            cycles += 8;
            break;
        case 0x74:
            gb->writeMemory(registers.hl, registers.h);
            cycles += 8;
            break;
        case 0x75:
            gb->writeMemory(registers.hl, registers.l);
            cycles += 8;
            break;

        case 0x76: //HALT
            HALT = true;
            cycles += 4;
            break;

        case 0x77: //LD [HL] A
            gb->writeMemory(registers.hl, registers.a);
            cycles += 8;
            break;

        case 0x78: //LD A r8
            registers.a = registers.b;
            cycles += 4;
            break;
        case 0x79: 
            registers.a = registers.c;
            cycles += 4;
            break;
        case 0x7A: 
            registers.a = registers.d;
            cycles += 4;
            break;
        case 0x7B: 
            registers.a = registers.e;
            cycles += 4;
            break;
        case 0x7C: 
            registers.a = registers.h;
            cycles += 4;
            break;
        case 0x7D: 
            registers.a = registers.l;
            cycles += 4;
            break;
        case 0x7E: //LD A [HL]
            registers.a = gb->readMemory(registers.hl);
            cycles += 8;
            break;
        case 0x7F: //LD A A
            registers.a = registers.a;
            cycles += 4;
            break;
        
        case 0x80:{//ADD A B
            uint16_t result = registers.a + registers.b;
            bool h = ((registers.a & 0x0F) + (registers.b & 0x0F))> 0x0F;
            registers.a = (uint8_t) (result & 0xFF);
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', h);
            setFlag('c', result > 255);
            cycles += 4;
            break;
        }

        case 0x81:{//ADD A C
            uint16_t result = registers.a + registers.c;
            bool h = ((registers.a & 0x0F) + (registers.c & 0x0F))> 0x0F;
            registers.a = (uint8_t) (result & 0xFF);
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', h);
            setFlag('c', result > 255);
            cycles += 4;
            break;
        }

        case 0x90:{//SUB A B
            uint8_t result = registers.a - registers.b;
            setFlag('z', result == 0);
            setFlag('n', true);
            setFlag('h', (registers.a & 0x0F) < (registers.b & 0x0F));
            setFlag('c', registers.b > registers.a);
            registers.a = result;
            cycles += 4;
            break;
        }

        case 0x91:{//SUB A C
            uint8_t result = registers.a - registers.c;
            setFlag('z', result == 0);
            setFlag('n', true);
            setFlag('h', (registers.a & 0x0F) < (registers.c & 0x0F));
            setFlag('c', registers.c > registers.a);
            registers.a = result;
            cycles += 4;
            break;
        }

        case 0xA8: //XOR A r8
            registers.a ^= registers.b;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;
        case 0xA9: 
            registers.a ^= registers.c;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;
        case 0xAA: 
            registers.a ^= registers.d;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;
        case 0xAB: 
            registers.a ^= registers.e;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;
        case 0xAC: 
            registers.a ^= registers.h;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;
        case 0xAD: 
            registers.a ^= registers.l;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;

        case 0xAE: //XOR A [HL]
            registers.a ^= gb->readMemory(registers.hl);
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 8;
            break;

        case 0xAF: //XOR A A
            registers.a = 0;
            setFlag('z', true);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;
        
        case 0xB0: //OR A r8
            registers.a |= registers.b;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;
        case 0xB1:
            registers.a |= registers.c;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;
        case 0xB2:
            registers.a |= registers.d;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;
        case 0xB3:
            registers.a |= registers.e;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;
        case 0xB4:
            registers.a |= registers.h;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;
        case 0xB5:
            registers.a |= registers.l;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;

        case 0xB6: //OR A [HL]
            registers.a |= gb->readMemory(registers.hl);
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 8;
            break;

        case 0xB7: //OR A A
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 4;
            break;

        case 0xB8:{//CP A B
            uint8_t result = registers.a - registers.b;
            setFlag('z', result == 0);
            setFlag('n', true);
            setFlag('h', (registers.a & 0x0F) < (registers.b & 0x0F));
            setFlag('c', registers.b > registers.a);
            cycles += 4;
            break;
        }
        
        case 0xB9:{//CP A C
            uint8_t result = registers.a - registers.c;
            setFlag('z', result == 0);
            setFlag('n', true);
            setFlag('h', (registers.a & 0x0F) < (registers.c & 0x0F));
            setFlag('c', registers.c > registers.a);
            cycles += 4;
            break;
        }

        case 0xBA:{//CP A D
            uint8_t result = registers.a - registers.d;
            setFlag('z', result == 0);
            setFlag('n', true);
            setFlag('h', (registers.a & 0x0F) < (registers.d & 0x0F));
            setFlag('c', registers.d > registers.a);
            cycles += 4;
            break;
        }

        case 0xBB:{//CP A E
            uint8_t result = registers.a - registers.e;
            setFlag('z', result == 0);
            setFlag('n', true);
            setFlag('h', (registers.a & 0x0F) < (registers.e & 0x0F));
            setFlag('c', registers.e > registers.a);
            cycles += 4;
            break;
        }
        
        case 0xC0: //RET NZ
            if (!getFlag('z')) {
                registers.pc = bytesToWord(gb->readMemory(registers.sp), gb->readMemory(registers.sp + 1));
                registers.sp += 2;
                cycles += 12;
            }
            cycles += 8;
            break;
        
        case 0xC1: //POP BC
            registers.c = gb->readMemory(registers.sp);
            registers.b = gb->readMemory(registers.sp + 1);
            registers.sp += 2;
            cycles += 12;
            break;
        
        case 0xC2:{//JP NZ nn
            uint16_t nn = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            if (!getFlag('z')) {
                registers.pc = nn;
                cycles += 4;
            }
            cycles += 12;
            break;
        }

        case 0xC3: //JP nn
            registers.pc = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            cycles += 16;
            break;
        
        case 0xC4:{//CALL NZ nn
            uint16_t nn = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            if (!getFlag('z')) {
                registers.sp--;
                gb->writeMemory(registers.sp, getMSB(registers.pc));
                registers.sp--;
                gb->writeMemory(registers.sp, getLSB(registers.pc));
                registers.pc = nn;
                cycles += 12;
            }
            cycles += 12;
            break;
        }

        case 0xC5: //PUSH BC
            registers.sp--;
            gb->writeMemory(registers.sp, registers.b);
            registers.sp--;
            gb->writeMemory(registers.sp, registers.c);
            cycles += 16;
            break;
        
        case 0xC6:{//ADD A n
            uint8_t n = gb->readMemory(registers.pc);
            registers.pc++;
            uint16_t result = registers.a + n;
            bool h = ((registers.a & 0x0F) + (n & 0x0F))> 0x0F;
            registers.a = (uint8_t) (result & 0xFF);
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', h);
            setFlag('c', result > 255);
            cycles += 8;
            break;
        }

        case 0xC8: //RET Z
            if (getFlag('z')) {
                registers.pc = bytesToWord(gb->readMemory(registers.sp), gb->readMemory(registers.sp + 1));
                registers.sp += 2;
                cycles += 12;
            }
            cycles += 8;
            break;
        
        case 0xC9: //RET
            registers.pc = bytesToWord(gb->readMemory(registers.sp), gb->readMemory(registers.sp + 1));
            registers.sp += 2;
            cycles += 16;
            break;

        case 0xCA:{//JP Z nn
            uint16_t nn = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            if (getFlag('z')) {
                registers.pc = nn;
                cycles += 4;
            }
            cycles += 12;
            break;
        }
        
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

        case 0xCE:{//ADC A n
            uint8_t n = gb->readMemory(registers.pc);
            registers.pc++;
            uint16_t result = registers.a + n + getFlag('c');
            bool h = ((registers.a & 0x0F) + (n & 0x0F) + getFlag('c'))> 0x0F;
            registers.a = (uint8_t) (result & 0xFF);
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', h);
            setFlag('c', result > 255);
            cycles += 8;
            break;
        }

        case 0xD0: //RET NC
            if (!getFlag('c')) {
                registers.pc = bytesToWord(gb->readMemory(registers.sp), gb->readMemory(registers.sp + 1));
                registers.sp += 2;
                cycles += 12;
            }
            cycles += 8;
            break;

        case 0xD1: //POP DE
            registers.e = gb->readMemory(registers.sp);
            registers.d = gb->readMemory(registers.sp + 1);
            registers.sp += 2;
            cycles += 12;
            break;
        
        case 0xD4:{//CALL NC nn
            uint16_t nn = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            if (!getFlag('c')) {
                registers.sp--;
                gb->writeMemory(registers.sp, getMSB(registers.pc));
                registers.sp--;
                gb->writeMemory(registers.sp, getLSB(registers.pc));
                registers.pc = nn;
                cycles += 12;
            }
            cycles += 12;
            break;
        }

        case 0xD5: //PUSH DE
            registers.sp--;
            gb->writeMemory(registers.sp, registers.d);
            registers.sp--;
            gb->writeMemory(registers.sp, registers.e);
            cycles += 16;
            break;

        case 0xD6:{//SUB A n
            uint8_t n = gb->readMemory(registers.pc);
            registers.pc++;
            uint8_t result = registers.a - n;
            setFlag('z', result == 0);
            setFlag('n', true);
            setFlag('h', (registers.a & 0x0F) < (n & 0x0F));
            setFlag('c', n > registers.a);
            registers.a = result;
            cycles += 8;
            break;
        }

        case 0xD8: //RET C
            if (getFlag('c')) {
                registers.pc = bytesToWord(gb->readMemory(registers.sp), gb->readMemory(registers.sp + 1));
                registers.sp += 2;
                cycles += 12;
            }
            cycles += 8;
            break;

        case 0xD9: //RETI
            registers.pc = bytesToWord(gb->readMemory(registers.sp), gb->readMemory(registers.sp + 1));
            registers.sp += 2;
            IME = true;
            cycles += 16;
            break;

        case 0xDA:{//JP C nn
            uint16_t nn = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            if (getFlag('c')) {
                registers.pc = nn;
                cycles += 4;
            }
            cycles += 12;
            break;
        }

        case 0xDE:{//SBC A n
            uint8_t n = gb->readMemory(registers.pc);
            registers.pc++;
            int result = registers.a - n - getFlag('c');
            bool h = ((registers.a & 0x0F) - (n & 0x0F) - getFlag('c')) < 0;
            registers.a = (uint8_t) (result & 0xFF);
            setFlag('z', registers.a == 0);
            setFlag('n', true);
            setFlag('h', h);
            setFlag('c', result < 0);
            cycles += 8;
            break;
        }

        case 0xE0: //LDH [a8] A
            gb->writeMemory(bytesToWord(gb->readMemory(registers.pc), 0xFF), registers.a);
            registers.pc++;
            cycles += 12;
            break;
        
        case 0xE1: //POP HL
            registers.l = gb->readMemory(registers.sp);
            registers.h = gb->readMemory(registers.sp + 1);
            registers.sp += 2;
            cycles += 12;
            break;
        
        case 0xE5: //PUSH HL
            registers.sp--;
            gb->writeMemory(registers.sp, registers.h);
            registers.sp--;
            gb->writeMemory(registers.sp, registers.l);
            cycles += 16;
            break;
        
        case 0xE6://AND A n
            registers.a &= gb->readMemory(registers.pc);
            registers.pc++;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', true);
            setFlag('c', false);
            cycles += 8;
            break;

        case 0xE8:{//ADD SP e8
            int8_t e = (int8_t) gb->readMemory(registers.pc);
            registers.pc++;
            uint32_t sp = getLSB(registers.sp);
            bool h = ((sp & 0x0F) + ((uint8_t)e & 0x0F))> 0x0F;
            bool c = ((sp & 0xFF) + ((uint8_t)e & 0xFF))> 0xFF;
            registers.sp += e;
            setFlag('z', false);
            setFlag('n', false);
            setFlag('h', h);
            setFlag('c', c);
            cycles += 16;
            break;
        }
        
        case 0xE9: //JP HL
            registers.pc = registers.hl;
            cycles += 4;
            break;

        case 0xEA:{//LD [a16] A
            uint16_t nn = bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1));
            registers.pc += 2;
            gb->writeMemory(nn, registers.a);
            cycles += 16;
            break;
        }

        case 0xEE: //XOR A n
            registers.a ^= gb->readMemory(registers.pc);
            registers.pc++;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 8;
            break;

        case 0xF0: //LDH A [a8]
            registers.a = gb->readMemory(bytesToWord(gb->readMemory(registers.pc), 0xFF));
            registers.pc++;
            cycles += 12;
            break;

        case 0xF1: //POP AF
            registers.f = gb->readMemory(registers.sp) & 0xF0;
            registers.a = gb->readMemory(registers.sp + 1);
            registers.sp += 2;
            cycles += 12;
            break;

        case 0xF3: //DI
            IME = false;
            ei_delay = 0;
            cycles += 4;
            break;
        
        case 0xF5: //PUSH AF
            registers.sp--;
            gb->writeMemory(registers.sp, registers.a);
            registers.sp--;
            gb->writeMemory(registers.sp, registers.f);
            cycles += 16;
            break;

        case 0xF6: //OR A n
            registers.a |= gb->readMemory(registers.pc);
            registers.pc++;
            setFlag('z', registers.a == 0);
            setFlag('n', false);
            setFlag('h', false);
            setFlag('c', false);
            cycles += 8;
            break;
        
        case 0xF8:{//LD HL, SP + e
            int8_t e = (int8_t) gb->readMemory(registers.pc);
            uint32_t sp = getLSB(registers.sp);
            bool h = ((sp & 0x0F) + ((uint8_t)e & 0x0F))> 0x0F;
            bool c = ((sp & 0xFF) + ((uint8_t)e & 0xFF))> 0xFF;
            registers.pc++;
            registers.hl = registers.sp + e;

            setFlag('z', false);
            setFlag('n', false);
            setFlag('h', h);
            setFlag('c', c);
            cycles += 12;
            break;
        }

        case 0xF9: //LD SP HL
            registers.sp = registers.hl;
            cycles += 8;
            break;

        case 0xFA: //LD A [a16]
            registers.a = gb->readMemory(bytesToWord(gb->readMemory(registers.pc), gb->readMemory(registers.pc + 1)));
            registers.pc += 2;
            cycles += 16;
            break;
        
        case 0xFB: //EI
            ei_delay = 2;
            cycles += 4;
            break;

        case 0xFE:{//CP A n
            uint8_t n = gb->readMemory(registers.pc);
            uint8_t result = registers.a - n;
            registers.pc ++;
            setFlag('z', result == 0);
            setFlag('n', true);
            setFlag('h', (registers.a & 0x0F) < (n & 0x0F));
            setFlag('c', n > registers.a);
            cycles += 8;
            break;
        }

        case 0xFF: //RST 0x38
            registers.sp--;
            gb->writeMemory(registers.sp, getMSB(registers.pc));
            registers.sp--;
            gb->writeMemory(registers.sp, getLSB(registers.pc));
            registers.pc = bytesToWord(0x38, 0x00);
            cycles += 16;
            break;

        default:
            std::cout<<std::hex<<(int)registers.ir<<"\n";
            break;
    }
    
    if (ei_delay > 0) {
        ei_delay--;
        if (ei_delay == 0) {
            IME = true;
        }
    }

    for (int i = 0; i < cycles / 4; i++) {
        timer.increment(); 

        gb->memory[0xFF04] = getMSB(timer.sys_clock);
        gb->memory[0xFF05] = timer.TIMA;
        gb->memory[0xFF06] = timer.TMA;
        gb->memory[0xFF07] = timer.TAC;
    }

    return cycles;
}

int Cpu::handleInterrupts() {
    uint8_t IF = gb->readMemory(0xFF0F);

    if ((registers.ie & IF) && HALT) { //HALT handler
        HALT = false;
    }


    int cycles = 0;
    if (IME) {
        if (registers.ie & IF) {
            if (getBit(registers.ie, 0) & getBit(IF, 0)) {//V-Blank Interrupt
                registers.sp--;
                gb->writeMemory(registers.sp, getMSB(registers.pc));
                registers.sp--;
                gb->writeMemory(registers.sp, getLSB(registers.pc));
                registers.pc = 0x40;
                IME = false;
                clearBit(IF, 0);
                gb->writeMemory(0xFF0F, IF);
                cycles += 20;
            }
            if (getBit(registers.ie, 1) & getBit(IF, 1)) {//LCD Interrupt
                registers.sp--;
                gb->writeMemory(registers.sp, getMSB(registers.pc));
                registers.sp--;
                gb->writeMemory(registers.sp, getLSB(registers.pc));
                registers.pc = 0x48;
                IME = false;
                clearBit(IF, 1);
                gb->writeMemory(0xFF0F, IF);
                cycles += 20;
            }
            if (getBit(registers.ie, 2) & getBit(IF, 2)) {//Timer Interrupt
                registers.sp--;
                gb->writeMemory(registers.sp, getMSB(registers.pc));
                registers.sp--;
                gb->writeMemory(registers.sp, getLSB(registers.pc));
                registers.pc = 0x50;
                IME = false;
                clearBit(IF, 2);
                gb->writeMemory(0xFF0F, IF);
                cycles += 20;
            }
            if (getBit(registers.ie, 3) & getBit(IF, 3)) {//Serial Interrupt
                registers.sp--;
                gb->writeMemory(registers.sp, getMSB(registers.pc));
                registers.sp--;
                gb->writeMemory(registers.sp, getLSB(registers.pc));
                registers.pc = 0x58;
                IME = false;
                clearBit(IF, 3);
                gb->writeMemory(0xFF0F, IF);
                cycles += 20;
            }
            if (getBit(registers.ie, 4) & getBit(IF, 4)) {//Joypad Interrupt
                registers.sp--;
                gb->writeMemory(registers.sp, getMSB(registers.pc));
                registers.sp--;
                gb->writeMemory(registers.sp, getLSB(registers.pc));
                registers.pc = 0x60;
                IME = false;
                clearBit(IF, 4);
                gb->writeMemory(0xFF0F, IF);
                cycles += 20;
            }
        }

        for (int i = 0; i < cycles; i++) {
            timer.increment(); 

            gb->memory[0xFF04] = getMSB(timer.sys_clock);
            gb->memory[0xFF05] = timer.TIMA;
            gb->memory[0xFF06] = timer.TMA;
            gb->memory[0xFF07] = timer.TAC;
        }
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