#include <iostream>
#include <mbc.h>

class Mbc0 : public Mbc {
    public:
        Mbc0(std::vector<uint8_t> romData) : Mbc(std::move(romData)) {}

        uint8_t readRom(uint16_t address) override {
            return rom[address];
        }
        void writeRom(uint16_t address, uint8_t value) override {

        }
};

std::unique_ptr<Mbc> init_mbc(std::vector<uint8_t> romData) {
    uint8_t cart_type = romData[0x0147];
    switch (cart_type) {
        case 0:
            return std::make_unique<Mbc0>(std::move(romData));
        case 1:

            break;
        case 2:

            break;
        case 3:

            break;
        default:
            std::cout<<"Missing Cartridge Type: "<<std::hex<<(int)cart_type;
    }
    return std::make_unique<Mbc0>(std::move(romData));
}