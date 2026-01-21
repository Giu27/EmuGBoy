//Copyright (C) 2026  Giuseppe Caruso

#include <Timer.h>
#include <utils.h>

Timer::Timer(Cpu* parent) : cpu(parent) {

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
}

void Timer::writeRegisters(uint16_t address, uint8_t value){
    switch (address) {
    case 0xFF04: // DIV,  bits of sys_clock
        changeSystemClock(0);
        break;
    case 0xFF05: // TIMA, the timer counter
        if (!TIMA_reload_cycle) TIMA = value;
        // Rare case where the Interrupt request can be aborted
        if (cycles_til_TIMA_IRQ == 1) cycles_til_TIMA_IRQ = 0;
        break;
    case 0xFF06: // TMA, the timer modulo
        // "If TMA is written the same cycle it is loaded to TIMA, TIMA is also loaded with that value."
        if (TIMA_reload_cycle) TIMA = value;
        TMA = value;
        break;
    case 0xFF07: {
        // TAC, the timer control
        bool old_bit = last_bit;
        last_bit &= ((value & 4) >> 2);

        detectEdge(old_bit, last_bit);
        TAC = value;
        break; }
    }
}