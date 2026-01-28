//Copyright (C) 2026  Giuseppe Caruso
#include <ppu.h>

Ppu::Ppu() {
    LCDC = 0x91;
    STAT = 0x85;
}

void Ppu::tick(int cycles) {

}