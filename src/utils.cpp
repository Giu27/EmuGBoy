//Copyright (C) 2026  Giuseppe Caruso
#include <utils.h>

bool getBit(uint8_t b, int pos) {
    return (b >> pos) & 1;
}

void setBit(uint8_t &b, int pos) {
    b |= (1 << pos);
}

void clearBit(uint8_t &b, int pos) {
    b &= ~(1 << pos);
}

uint16_t bytesToWord(uint8_t lsb, uint8_t msb) {
     return ((uint16_t)msb << 8) | lsb;
}