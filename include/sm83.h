//Copyright (C) 2026  Giuseppe Caruso
#pragma once

#include <cstdint>

class Gb;

union RegisterPair { // Many registers can be accessed as one 16 bits register or as two 8 bits halves.
    uint16_t reg;
    struct {
        uint8_t l;
        uint8_t h;
    };
};

class Cpu {
    public:
        struct Registers { // SM83 Registers
            uint16_t pc; // Program Counter
            uint16_t sp; // Stack Pointer
            uint8_t ie; // Interrupt Enable
            uint8_t ir; // Instruction Register

            RegisterPair AF; // Accumulator and Flag
            RegisterPair BC; // General registers
            RegisterPair DE;
            RegisterPair HL;
            
            // Helpers for cleaner syntax
            uint8_t& a = AF.h;
            uint8_t& f = AF.l;
            uint8_t& b = BC.h;
            uint8_t& c = BC.l;
            uint8_t& d = DE.h;
            uint8_t& e = DE.l;
            uint8_t& h = HL.h;
            uint8_t& l = HL.l;

            uint16_t& af = AF.reg;
            uint16_t& bc = BC.reg;
            uint16_t& de = DE.reg;
            uint16_t& hl = HL.reg;
        } registers = {0};
    
        Cpu(Gb* parent);
        int step();
        bool getFlag(char flag);
        void setFlag(char flag, bool val);

    private: 
        Gb* gb;
};