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
