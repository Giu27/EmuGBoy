//Copyright (C) 2026  Giuseppe Caruso

//Took a lot of inspiration from @raddad772. Thanks!

#pragma once

#include <cstdint>

class Cpu;

class Timer {
    public:
        uint16_t sys_clock; //System Clock, top 8 bits are read from the DIV register.
        uint8_t TIMA; //FF05
        uint8_t TMA; //FF06
        uint8_t TAC; //FF07

        bool last_bit;
        bool TIMA_reload_cycle;
        int cycles_til_TIMA_IRQ;

        Timer(Cpu* parent);
        uint8_t readRegisters(uint16_t address);
        void writeRegisters(uint16_t address, uint8_t value);
        void increment();
        void detectEdge(bool before, bool after);
        void changeSystemClock(uint16_t value);
    private:
        Cpu* cpu;
};