//Copyright (C) 2026  Giuseppe Caruso
#include <iostream>
#include <fstream>
#include <utils.h>
#include <gb.h>

Gb::Gb() : cpu(this){
    
}

void Gb::loadRom(std::string path) { //Reads bytes from the rom and load it in memory. For now only handles no MBCs
    std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file){
        std::cerr<< "Error loading the ROM!";
        return ;
    }

    std::streamsize size = file.tellg();
    char buffer[size];

    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
	file.close();

    for (unsigned int i = 0; i < size; i++) {
		memory[i] = buffer[i];
	}
}

uint8_t Gb::readMemory(uint16_t addr) {
    return memory[addr]; //Temporary, will need to be replaced by a proper handling
}

void Gb::writeMemory(uint16_t addr, uint8_t value) {
    memory[addr] = value; //Temporary, will need to be replaced by a proper handling
}