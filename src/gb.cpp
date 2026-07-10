//Copyright (C) 2026  Giuseppe Caruso
#include <iostream>
#include <vector>
#include <fstream>
#include <utils.h>
#include <gb.h>

Gb::Gb() : cpu(this), ppu(this){
    dma_t_clocks = 0;
    dma_byte_index = 0;
}

void Gb::loadBootRom(std::string path) { //Reads bytes from the bootrom and load it
    std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file){
        std::cerr<< "Error loading the Boot ROM!";
        boot_rom_mapped = false;
        return ;
    }

    cpu.registers.pc = 0;

    std::streamsize size = file.tellg();
    std::vector<char> buffer(size);

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
	file.close();

    for (unsigned int i = 0; i < size; i++) {
		boot_rom[i] = buffer[i];
	}
}

void Gb::loadRom(std::string path) { //Reads bytes from the rom and load it in memory. For now only handles no MBCs
    std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file){
        std::cerr<< "Error loading the ROM!";
        return ;
    }

    std::streamsize size = file.tellg();
    std::vector<char> buffer(size);

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
	file.close();

    for (unsigned int i = 0; i < size; i++) {
		memory[i] = buffer[i];
	}

    if (memory[0x014D] != 0x00) {
        cpu.setFlag('c', true);
        cpu.setFlag('h', true);
    }
}

void Gb::updateJoypad() {
    uint8_t joyp = memory[0xFF00] | 0x0F; 
    uint8_t old_joyp = memory[0xFF00];

    // 0: A, 1: B, 2: select, 3: start, 4: UP, 5: DOWN, 6: LEFT, 7: RIGHT
    
    if (!getBit(joyp, 5)) {
        if (keystate[0]) clearBit(joyp, 0); // A
        if (keystate[1]) clearBit(joyp, 1); // B
        if (keystate[2]) clearBit(joyp, 2); // Select
        if (keystate[3]) clearBit(joyp, 3); // Start
    }

    // Check Direction Keys (Bit 4 is 0)
    if (!getBit(joyp, 4)) {
        if (keystate[7]) clearBit(joyp, 0); // Right
        if (keystate[6]) clearBit(joyp, 1); // Left
        if (keystate[4]) clearBit(joyp, 2); // Up
        if (keystate[5]) clearBit(joyp, 3); // Down
    }

    memory[0xFF00] = joyp;

    for (int i = 0; i < 4; i++) {
        if (getBit(old_joyp, i) && !getBit(joyp, i)) {
            setBit(memory[0xFF0F], 4); 
            break;
        }
    }
}

uint8_t Gb::readMemory(uint16_t addr) {
    if (DMATR) {
        if (addr < 0xFF00 || addr > 0xFFFF) {
            return 0xFF;
        }
    }
    //if (addr == 0xFF44) return 0x90; //Stub LY register, useful for test roms
    if (addr == 0xFF00) {
        updateJoypad();
    }
    
    if (addr <= 0xFF && boot_rom_mapped) {
        return boot_rom[addr];
    }

    if (addr >= 0xFF40 && addr <= 0xFF41 || addr == 0xFF44) {
        return ppu.readMemory(addr);
    }

    if (addr >= 0xFF04 && addr <= 0xFF07){
        return cpu.timer.readRegisters(addr);
    }

    if (addr == 0xFF0F){
        return memory[addr] | 0xE0;
    }

    if (addr == 0xFFFF) return cpu.registers.ie;
    return memory[addr]; 
}

void Gb::writeMemory(uint16_t addr, uint8_t value) {
    if (DMATR) {
        if (addr < 0xFF00 || addr > 0xFFFF) {
            return;
        }
    }
    if (addr <= 0x7FFF) {
        return;
    }
    if (addr == 0xFF00) {
        memory[addr] = 0xCF | (value & 0x30);
        updateJoypad();
        return;
    }

    if (addr == 0xFF02 && value == 0x81) { //Intercepts serial output
        std::cout<<(char)memory[0xFF01];
        value &= 0x7F;
    }

    if (addr == 0xFF50) {
        boot_rom_mapped = false;
        return;
    }

    if (addr >= 0xFF40 && addr <= 0xFF41 || addr == 0xFF44) {
        ppu.writeMemory(addr, value);
        return;
    }

    if (addr >= 0xFF04 && addr <= 0xFF07){
        cpu.timer.writeRegisters(addr, value);
        return;
    }

    if (addr == 0xFFFF) { //IE Register
        cpu.registers.ie = value;
        return;
    }

    if (addr >= 0xC000 && addr <= 0xDDFF) { //Echoes in echo RAM
        memory[addr + 0x2000] = value;
    }

    if (addr >= 0xE000 && addr <= 0xFDFF) { //Echoes in echo RAM
        memory[addr - 0x2000] = value;
    }

    if (addr == 0xFF46) { //DMA transfer
        std::cout << "DMA triggered! Source high byte: 0x" << std::hex << (int)value << std::endl;
        memory[addr] = value;
        DMATR= true;
        dma_source = value;
        return;
    }

    memory[addr] = value;
}