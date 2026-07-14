#include <iostream>
#include <mbc.h>

class Mbc0 : public Mbc {
    public:
        Mbc0(std::vector<uint8_t> rom_data) : Mbc(std::move(rom_data), false) {}

        uint8_t readRom(uint16_t addr) override {
            return rom[addr];
        }

        void writeRom(uint16_t addr, uint8_t value) override {

        }
};

class Mbc1 : public Mbc {
    private:
        uint8_t rom_bank = 0;
        uint8_t ram_bank = 0;
        uint8_t banking_mode = 0;
    public:
        Mbc1(std::vector<uint8_t> rom_data, bool battery) : Mbc(std::move(rom_data), battery) {}

        uint8_t readRom(uint16_t addr) override {
            if (addr <= 0x3FFF) {
                return rom[addr];
            }

            uint8_t actual_bank = rom_bank;
            if (banking_mode == 0) {
                actual_bank |= (ram_bank << 5);
            }

            uint32_t real_addr = (actual_bank * 0x4000) + (addr - 0x4000);
            return rom[real_addr % rom.size()];
        }

        void writeRom(uint16_t addr, uint8_t value) override {
            if (addr <= 0x1FFF) {
                ram_enabled = ((value & 0x0F) == 0xA);
            } else if (addr >= 0x2000 && addr <= 0x3FFF) {
                rom_bank = value & 0x1F; 
                if (rom_bank == 0) rom_bank = 1; 
            } else if (addr >= 0x4000 && addr <= 0x5FFF) {
                ram_bank = value & 0x03; 
            } else if (addr >= 0x6000 && addr <= 0x7FFF) {
                banking_mode = value & 0x01; 
            }
        }

        uint8_t readRam(uint16_t addr) override {
            if (ram_enabled && !ram.empty()) {
                uint8_t actual_ram_bank = (banking_mode == 1) ? ram_bank : 0;
                uint32_t real_addr = (actual_ram_bank * 0x2000) + (addr - 0xA000);
                return ram[real_addr % ram.size()];
            } else return 0xFF;
        }

        void writeRam(uint16_t addr, uint8_t value) override {
            if (ram_enabled&& !ram.empty()) {
                uint8_t actual_ram_bank = (banking_mode == 1) ? ram_bank : 0;
                uint32_t real_addr = (actual_ram_bank * 0x2000) + (addr - 0xA000);
                ram[real_addr % ram.size()] = value;
            }
        }
};

std::unique_ptr<Mbc> init_mbc(std::vector<uint8_t> rom_data) {
    uint8_t cart_type = rom_data[0x0147];
    switch (cart_type) {
        case 0:
            return std::make_unique<Mbc0>(std::move(rom_data));
        case 1:
            return std::make_unique<Mbc1>(std::move(rom_data), false);
            break;
        case 3:
            return std::make_unique<Mbc1>(std::move(rom_data), true);
            break;
        default:
            std::cout<<"Missing Cartridge Type: "<<std::hex<<(int)cart_type;
    }
    return std::make_unique<Mbc0>(std::move(rom_data));
}