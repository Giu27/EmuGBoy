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
}

void Ppu::loadBackGround() {
    uint16_t base_pointer = getBit(LCDC, 4) ? 0x8000 : 0x9000;
    uint16_t tile_map = getBit(LCDC, 3) ? 0x9C00 : 0x9800;
    uint16_t addr;
    uint8_t colour;

    SCY = gb->readMemory(0xFF42);
    SCX = gb->readMemory(0xFF43);

    uint8_t pal = gb->readMemory(0xFF47); //BGP
    uint8_t offY = LY + SCY;
    for (int i = 0; i < 160; i++) {
        uint8_t offX = i + SCX;

        uint8_t tile_idx = gb->readMemory(tile_map + ((offY / 8 * 32) + (offX / 8))); 

        if (base_pointer == 0x8000) {
            addr = base_pointer + (tile_idx * 16) + (offY % 8 * 2);
            colour = (getBit(gb->readMemory(addr), 7- (offX % 8))) + (getBit(gb->readMemory(addr + 1), 7- (offX % 8))) * 2;
        } else {
            addr = base_pointer + ((int8_t) tile_idx * 16) + (offY % 8 * 2);
            colour = (getBit(gb->readMemory(addr), 7- (offX % 8))) + (getBit(gb->readMemory(addr + 1), 7- (offX % 8))) * 2;
        }

        uint32_t colorfrompal = (pal >> (2 * colour)) & 3;
        frame_buffer[LY * 160 + i] = palette[colorfrompal];
    }
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
    STAT |= current_mode;
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
                }
                gb->memory[0xFF44] = LY;
                checkLYC();
            }
            break;

        case MODE2_OAM:
            if (dots >= 80){
                current_mode = MODE3_DRAW;
                dots -= 80;
            }        
            break;

        case MODE3_DRAW:
            loadBackGround();
            if (dots >= 172) {
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