#pragma once

#include <sm83.h>

#define MEMORY_SIZE 0x10000

class Gb{
    public:
        Cpu cpu;
        uint8_t memory [MEMORY_SIZE] = {0};
};