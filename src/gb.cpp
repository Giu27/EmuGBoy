//Copyright (C) 2026  Giuseppe Caruso
#include <iostream>
#include <vector>
#include <fstream>
#include <utils.h>
#include <gb.h>

Gb::Gb() : cpu(this), ppu(this){
    
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

uint8_t Gb::readMemory(uint16_t addr) {
    //if (addr == 0xFF44) return 0x90; //Stub LY register, useful for test roms

    if (addr >= 0xFF04 && addr <= 0xFF07){
        return cpu.timer.readRegisters(addr);
    }

    if (addr == 0xFFFF) return cpu.registers.ie;
    return memory[addr]; //Temporary, will need to be replaced by a proper handling
}

void Gb::writeMemory(uint16_t addr, uint8_t value) {
    if (addr == 0xFF02 && value == 0x81) { //Intercepts serial output
        std::cout<<(char)memory[0xFF01];
        value &= 0x7F;
    }

    if (addr >= 0xFF04 && addr <= 0xFF07){
        cpu.timer.writeRegisters(addr, value);
        return;
    }

    if (addr == 0xFFFF) { //IE Register
        cpu.registers.ie = value;
    }

    if (addr >= 0xC000 && addr <= 0xDDFF) { //Echoes in echo RAM
        memory[addr + 0x2000] = value;
    }

    memory[addr] = value; //Temporary, will need to be replaced by a proper handling
}