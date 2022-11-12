#ifdef POLYCONTROL

#include "storageDataStructure.hpp"
#include "datacore/datalocation.hpp"
#include "hardware/device.hpp"

extern M95M01 eeprom;

RAM1 uint8_t blockBuffer[BLOCKBUFFERSIZE];

RAM3_DMA volatile uint8_t presetDMABlockBuffer[BLOCKBUFFERSIZE];

void writeConfigBlock() {
    eeprom.SPI_WriteBuffer(blockBuffer, CONFIG_STARTADDRESS, CONFIG_BLOCKSIZE);
}

void writePresetTableBlock() {
    eeprom.SPI_WriteBuffer(blockBuffer, TABLE_STARTADDRESS, TABLE_BLOCKSIZE);
}

void writePresetBlock(uint32_t presetID) {
    uint32_t address = presetID * PRESET_BLOCKSIZE + PRESET_STARTADDRESS;
    eeprom.SPI_WriteBuffer(blockBuffer, address, PRESET_BLOCKSIZE);
}

uint8_t *readPresetBlock(uint32_t presetID) {

    uint32_t address = presetID * PRESET_BLOCKSIZE + PRESET_STARTADDRESS;

    eeprom.SPI_ReadBuffer(blockBuffer, address, PRESET_BLOCKSIZE);

    return blockBuffer;
}

uint8_t *readPresetTableBlock() {

    eeprom.SPI_ReadBuffer(blockBuffer, TABLE_STARTADDRESS, TABLE_BLOCKSIZE);

    return blockBuffer;
}

uint8_t *readConfigBlock() {

    eeprom.SPI_ReadBuffer(blockBuffer, CONFIG_STARTADDRESS, CONFIG_BLOCKSIZE);

    return blockBuffer;
}

void clearEEPROM() { // CLEAR EEPROM WHEN CORRUPTED

    println("INFO | CLEAR EEPROM .... START");

    uint8_t buffer[2048];
    uint32_t data = 0x00;

    fastMemset((uint32_t *)buffer, &data, 2048 / 4);

    for (uint32_t i = 0; i < EEPROM_SIZE / 2048; i++) {
        eeprom.SPI_WriteBuffer(buffer, 2048 * i, 2048);
    }
    println("   ... DONE");
}
#endif