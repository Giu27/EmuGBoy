//Copyright (C) 2026  Giuseppe Caruso

#include <iostream>
#include <utils.h>
#include <sm83.h>
#include <gb.h>
#include <Timer.h>

Timer::Timer(Cpu* parent) : cpu(parent) {
    sys_clock = 0xAB00;
    TAC = 0xF8;
    TMA = 0;
    TIMA = 0;
}

uint8_t Timer::readRegisters(uint16_t address) {
    switch (address) {
        case 0xFF04: // DIV, upper 8 bits of sys_clock 
            return getMSB(sys_clock);
        case 0xFF05:
            return TIMA;
        case 0xFF06:
            return TMA;
        case 0xFF07:
            return TAC;
    }
    return 0xFF;
}

void Timer::writeRegisters(uint16_t address, uint8_t value) {
    switch (address) {
    case 0xFF04: // DIV, top 8 bits of sys_clock
        changeSystemClock(0);
        break;
    case 0xFF05: // TIMA
        if (!TIMA_reload_cycle) TIMA = value;
        // Rare case where the Interrupt request can be aborted
        if (cycles_til_TIMA_IRQ == 1) cycles_til_TIMA_IRQ = 0;
        cpu->gb->memory[0xFF05] = TIMA;
        break;
    case 0xFF06: //TMA
        if (TIMA_reload_cycle) TIMA = value;
        TMA = value;
        cpu->gb->memory[0xFF06] = TMA;
        break;
    case 0xFF07: {
        //TAC
        bool old_bit = last_bit;
        last_bit &= ((value & 4) >> 2);

        detectEdge(old_bit, last_bit);
        TAC = value;
        cpu->gb->memory[0xFF07] = TAC;
        break; }
    }
}

void Timer::detectEdge(bool before, bool after) {
    if (before && !after) {
        TIMA++; // Increment TIMA
        if (TIMA == 0) { // If we overflow, schedule IRQ
            cycles_til_TIMA_IRQ = 1;
        }
    }
}

void Timer::changeSystemClock(uint16_t new_value) {
    // 00 = bit 9, lowest speed, /1024  4096 hz   & 0x200
    // 01 = bit 3,               /16  262144 hz   & 0x08
    // 10 = bit 5,               /64  65536 hz    & 0x20
    // 11 = bit 7,              /256  16384 hz    & 0x80
    sys_clock = new_value;
    bool this_bit = 0;
    switch (TAC & 3) {
    case 0: // using bit 7
        this_bit = (sys_clock >> 7) & 1;
        break;
    case 3: // using bit 5
        this_bit = (sys_clock >> 5) & 1;
        break;
    case 2: // using bit 3
        this_bit = (sys_clock >> 3) & 1;
        break;
    case 1: // using bit 1
        this_bit = (sys_clock >> 1) & 1;
        break;
    }
    this_bit &= ((TAC & 4) >> 2);

    detectEdge(last_bit, this_bit);
    last_bit = this_bit;
}

void Timer::increment() {
    TIMA_reload_cycle = false;
    if (cycles_til_TIMA_IRQ > 0) {
        cycles_til_TIMA_IRQ--;
        if (cycles_til_TIMA_IRQ == 0) {
            setBit(cpu->gb->memory[0xFF0F], 2);
            TIMA = TMA;
            cpu->gb->memory[0xFF05] = TMA;
            TIMA_reload_cycle = true;
        }
    }
    changeSystemClock((sys_clock + 1));
}