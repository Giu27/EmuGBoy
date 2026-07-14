#pragma once

#include <cstdint>
#include <vector>
#include <memory>

class Mbc {
    public:
        std::vector<uint8_t> rom;
        std::vector<uint8_t> ram;

        Mbc(std::vector<uint8_t> romData) : rom(std::move(romData)) {}

        virtual ~Mbc() = default;

        virtual uint8_t readRom(uint16_t address) = 0;
        virtual void writeRom(uint16_t address, uint8_t value) = 0;
        virtual uint8_t readRam(uint16_t address) {return 0xFF;}
        virtual void writeRam(uint16_t address, uint8_t value) {}
};

std::unique_ptr<Mbc> init_mbc(std::vector<uint8_t> romData);