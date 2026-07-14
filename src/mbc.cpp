#include <iostream>
#include <mbc.h>

class Mbc0 : public Mbc {
    public:
        Mbc0(std::vector<uint8_t> romData) : Mbc(std::move(romData), false) {}

        uint8_t readRom(uint16_t address) override {
            return rom[address];
        }

        void writeRom(uint16_t address, uint8_t value) override {

        }
};

class Mbc1 : public Mbc {
    private:
        uint8_t rom_bank = 0;
        uint8_t ram_bank = 0;
        uint8_t banking_mode = 0;
    public:
        Mbc1(std::vector<uint8_t> romData, bool battery) : Mbc(std::move(romData), battery) {}

        uint8_t readRom(uint16_t address) override {

        }

        void writeRom(uint16_t address, uint8_t value) override {

        }

        uint8_t readRam(uint16_t address) override {

        }

        void writeRam(uint16_t address, uint8_t value) override {

        }
};

std::unique_ptr<Mbc> init_mbc(std::vector<uint8_t> romData) {
    uint8_t cart_type = romData[0x0147];
    switch (cart_type) {
        case 0:
            return std::make_unique<Mbc0>(std::move(romData));
        case 1:
            return std::make_unique<Mbc1>(std::move(romData), false);
            break;
        case 3:
            return std::make_unique<Mbc1>(std::move(romData), true);
            break;
        default:
            std::cout<<"Missing Cartridge Type: "<<std::hex<<(int)cart_type;
    }
    return std::make_unique<Mbc0>(std::move(romData));
}