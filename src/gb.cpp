#include <iostream>
#include <fstream>
#include <gb.h>

void Gb::load_rom(std::string path) { //Reads bytes from the rom and load it in memory. For now only handles no MBCs
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