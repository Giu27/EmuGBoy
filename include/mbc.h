#pragma once

#include <cstdint>
#include <vector>
#include <memory>

class Mbc {
    public:
        std::vector<uint8_t> rom;
        std::vector<uint8_t> ram;

        bool ramEnabled = false;
        bool hasBattery = false;

        Mbc(std::vector<uint8_t> romData, bool battery) : rom(std::move(romData)), hasBattery(battery) {
            uint32_t ramSize = 0;
            switch (rom[0x0149]) {
                case 0x01: ramSize = 0x800;   break; // 2 KB
                case 0x02: ramSize = 0x2000;   break; // 8 KB
                case 0x03: ramSize = 0x8000;  break; // 32 KB (4 banks of 8 KB)
                case 0x04: ramSize = 0x20000; break; // 128 KB (16 banks of 8 KB)
                case 0x05: ramSize = 0x10000; break; // 64 KB (8 banks of 8 KB)
                default:   ramSize = 0;      break; 
            }
        
            if (ramSize > 0) {
                ram.resize(ramSize, 0xFF);
            }

        }

        virtual ~Mbc() = default;

        virtual uint8_t readRom(uint16_t address) = 0;
        virtual void writeRom(uint16_t address, uint8_t value) = 0;
        virtual uint8_t readRam(uint16_t address) {return 0xFF;}
        virtual void writeRam(uint16_t address, uint8_t value) {}
};

std::unique_ptr<Mbc> init_mbc(std::vector<uint8_t> romData);