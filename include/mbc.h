#pragma once

#include <cstdint>
#include <vector>
#include <memory>

class Mbc {
    public:
        std::vector<uint8_t> rom;
        std::vector<uint8_t> ram;

        bool ram_enabled = false;
        bool has_battery = false;

        Mbc(std::vector<uint8_t> rom_data, bool battery) : rom(std::move(rom_data)), has_battery(battery) {
            uint32_t ram_size = 0;
            switch (rom[0x0149]) {
                case 0x01: ram_size = 0x800;   break; // 2 KB
                case 0x02: ram_size = 0x2000;   break; // 8 KB
                case 0x03: ram_size = 0x8000;  break; // 32 KB (4 banks of 8 KB)
                case 0x04: ram_size = 0x20000; break; // 128 KB (16 banks of 8 KB)
                case 0x05: ram_size = 0x10000; break; // 64 KB (8 banks of 8 KB)
                default:   ram_size = 0;      break; 
            }
        
            if (ram_size > 0) {
                ram.resize(ram_size, 0xFF);
            }

        }

        virtual ~Mbc() = default;

        virtual uint8_t readRom(uint16_t addr) = 0;
        virtual void writeRom(uint16_t addr, uint8_t value) = 0;
        virtual uint8_t readRam(uint16_t addr) {return 0xFF;}
        virtual void writeRam(uint16_t addr, uint8_t value) {}
};

std::unique_ptr<Mbc> init_mbc(std::vector<uint8_t> rom_data);