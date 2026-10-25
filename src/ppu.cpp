//Copyright (C) 2026  Giuseppe Caruso
#include <iostream>
#include <gb.h>
#include <utils.h>
#include <ppu.h>

Ppu::Ppu(Gb* parent) : gb(parent) {
    LCDC = 0x91;
    STAT = 0x85;
    current_mode = MODE2_OAM;
    dots = 0;
    WLY = 0;
    window_triggered = false;
}

void Ppu::loadBackGround() {
    LCDC = gb->readMemory(0xFF40);
    STAT = gb->readMemory(0xFF41);
    SCY = gb->readMemory(0xFF42);
    SCX = gb->readMemory(0xFF43);
    WY = gb->readMemory(0xFF4A);
    WX = gb->readMemory(0xFF4B);
    uint8_t pal = gb->readMemory(0xFF47);

    if (!getBit(LCDC, 0)) { //background not enabled
        for (int i = 0; i < 160; i++) {
            frame_buffer[LY * 160 + i] = palette[0];
        }
        return;
    }

    if (getBit(LCDC, 5) && LY == WY) window_triggered = true;
    bool window = getBit(LCDC, 5) && window_triggered && WX <= 166;
    bool window_drawn = false;

    bool unsigned_tiles = getBit(LCDC, 4);
    uint16_t bg_map  = getBit(LCDC, 3) ? 0x9C00 : 0x9800;
    uint16_t win_map = getBit(LCDC, 6) ? 0x9C00 : 0x9800;

    for (int i = 0; i < 160; i++) {
        uint16_t map;
        uint8_t x, y;
        if (window && i >= WX - 7) {
            map = win_map; 
            y = WLY; 
            x = i - (WX - 7);
            window_drawn = true;
        } else {
            map = bg_map; 
            y = LY + SCY; 
            x = i + SCX;
        }

        uint8_t tile = gb->readMemory(map + ((y >> 3) << 5) + (x >> 3)); // >> 3 << 5 = /8 * 32
        uint16_t addr = unsigned_tiles ? 0x8000 + tile * 16 : 0x9000 + static_cast<int8_t>(tile) * 16;
        addr += (y & 7) * 2;

        int bit = 7 - (x & 7);
        uint8_t colour = getBit(gb->readMemory(addr), bit)  | (getBit(gb->readMemory(addr + 1), bit) << 1);

        frame_buffer[LY * 160 + i] = palette[(pal >> (colour * 2)) & 3];
    }

    //Checks LY and WLY are synchronized,needed it for the bugged HUD of Tloz Link's Awakening 
    /*if (window_drawn && LY >= 120) {
        printf("LY=%d WY=%d WX=%d WLY=%d LCDC=%02X\n", LY, WY, WX, WLY, LCDC);
    }
    if (window_drawn && WLY != LY - WY) {
        printf("DESYNC: LY=%d WY=%d WLY=%d\n", LY, WY, WLY);
    }*/

    if (window_drawn) WLY++;
}

void Ppu::checkLYC() {
    uint8_t LYC = gb->readMemory(0xFF45);
    if (LY == LYC) {
        setBit(STAT, 2);
        gb->writeMemory(0xFF41, STAT);
        if (getBit(STAT, 6)) {
            setBit(gb->memory[0xFF0F], 1);
        }
    } else {
        clearBit(STAT, 2);
        gb->writeMemory(0xFF41, STAT);
    }
}

void Ppu::checkSTAT() {
    STAT = (STAT & ~0x03) | current_mode;
    gb->writeMemory(0xFF41, STAT);
    bool interrupt = false;
    if (current_mode == MODE0_HBLANK && getBit(STAT, 3)){
        interrupt = true;
    }
    if (current_mode == MODE1_VBLANK && getBit(STAT, 4)){
        interrupt = true;
    }
    if (current_mode == MODE2_OAM && getBit(STAT, 5)){
        interrupt = true;
    }
    if (interrupt) {
        setBit(gb->memory[0xFF0F], 1);
    }
}

void Ppu::tick(int cycles) {
    dots += cycles;
    
    switch(current_mode) { //TODO Properly handle mode 0 and mode 3 duration
        case MODE0_HBLANK:
            gb->OAM_block = false;
            if (dots >= 204) {
                LY++;
                gb->memory[0xFF44] = LY;
                checkLYC();
                if (LY == 144) {
                    current_mode = MODE1_VBLANK;
                    setBit(gb->memory[0xFF0F], 0);
                } else {
                    current_mode = MODE2_OAM; 
                }
                checkSTAT();   
                dots -= 204;  
            }
            break;

        case MODE1_VBLANK:
            if (dots >= 456) {
                dots -= 456;
                LY++;
                if (LY == 154) {
                    current_mode = MODE2_OAM;
                    checkSTAT();
                    LY = 0;
                    WLY = 0;
                    window_triggered = false;
                }
                gb->memory[0xFF44] = LY;
                checkLYC();
            }
            break;

        case MODE2_OAM:
            gb->OAM_block = true;
            if (dots >= 80){
                current_mode = MODE3_DRAW;
                dots -= 80;
            }        
            break;

        case MODE3_DRAW:
            if (LY == 0) {
                window_triggered = false;
            }

            if (dots >= 172) {
                loadBackGround();
                current_mode = MODE0_HBLANK; 
                checkSTAT();
                dots -= 172;
            }
            break;
    }
}

uint8_t Ppu::readMemory(uint16_t addr) {
    if (addr == 0xFF44) return LY;
    if (addr == 0xFF40) return LCDC;
    if (addr == 0xFF41) return STAT;
    return 0xFF;
}

void Ppu::writeMemory(uint16_t addr, uint8_t value) {
    if (addr == 0xFF44) return;
    if (addr == 0xFF40) {
        LCDC = value;
        gb->memory[0xFF40] = LCDC;
    }
    if (addr == 0xFF41) {
        STAT = value;
        gb->memory[0xFF41] = STAT;
    }
}