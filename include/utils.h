//Copyright (C) 2026  Giuseppe Caruso
#pragma once

#include <cstdint>

bool getBit(uint8_t b, int pos);

void setBit(uint8_t &b, int pos);

void clearBit(uint8_t &b, int pos);

uint16_t bytesToWord(uint8_t lsb, uint8_t msb);

uint8_t getLSB(uint16_t val);

uint8_t getMSB(uint16_t val);