
#ifdef POLYCONTROL
#include "supervisor/supervisor.hpp"

#include "polyControl.hpp"

#include "gfx/gui.hpp"
#include "globalsettings/globalSettings.hpp"
#include "hardware/device.hpp"
#include "humanInterface/hid.hpp"

#include "debughelper/firmware.hpp"
#include "storage/loadStoredData.hpp"
#include "usbd_cdc_if.hpp"

extern USBD_HandleTypeDef hUsbDeviceHS;
extern USBD_HandleTypeDef hUsbDeviceFS;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

extern devManager deviceManager;
extern GUI ui;

#define CHUNKSIZE 128 // usb limit?

void printHelp() {
    println("Hello Frank here!");
    println("Here some help for you:");
    println("-h    help");
    println("-r    system reset");
    println("-d    hardware report");
    println("-s    status report");
    println("-isp  flash all 4 render uC (COMmunicate)");
    println("-dfu  enable DFU mode (not working)");
    println("-EEPROM_CLEAR  clear EEPROM IC");
    println("-getPresetTable");
    println("-getPresetFromID");
    println("-writePresetTable");
    println("-writePresetToID");
}

void printStatus() {
    std::string report;
    globalSettings.statusReport(report);
    println(report);
}
void printDeviceManager() {
    std::string report;
    deviceManager.report(report);
    println(report);
}

void printPresetTable() {
    readPresetTable();
    CDC_Transmit_FS(blockBuffer, TABLE_BLOCKSIZE);
}

void printPreset(uint8_t index) {
    readPresetBlock(index);
    if (checkBuffer()) {
        CDC_Transmit_FS(blockBuffer, PRESET_BLOCKSIZE);
    }
}

void sendDataACK() {
    uint8_t res = DATAVALID;
    CDC_Transmit_FS(&res, 1);
}
void sendDataNACK() {
    uint8_t res = DATAINVALID;
    CDC_Transmit_FS(&res, 1);
}

extern USBD_HandleTypeDef hUsbDeviceHS;
extern USBD_HandleTypeDef hUsbDeviceFS;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
void COMmunicateISR() {
    static std::vector<char> data;
    static elapsedMillis timeout = 0;

    while (comAvailable()) {

        FlagHandler::COM_USB_TRAFFIC = true;
        FlagHandler::COM_PRINT_ENABLE = true; // reactivate USB COM seems we have an open com port

        if (data.size() > 50) { // take care we are not flooding our memory
            data.clear();
        }
        data.push_back(comRead());
        if (!data.empty()) { // data not empty -> decode

            if (data.back() == (char)10) { // if we get a "ENTER" decode the data
                data.pop_back();
                std::string command(data.begin(), data.end());

                if (command.compare("-dfu") == 0) {

                    // uint8_t response = ENTERMODE;
                    // CDC_Transmit_FS(&response, 1);

                    // RCC->AHB1ENR &= ~(RCC_AHB1ENR_USB1OTGHSEN | RCC_AHB1ENR_USB2OTGFSEN);

                    // HAL_Delay(500);

                    // rebootToBooloader();
                    // break;
                }
                else if (command.compare("-isp") == 0) { // make a restart to ISP

                    /////////Clear Polyfrank
                    __disable_irq();

                    HAL_NVIC_DisableIRQ(DMA1_Stream0_IRQn);
                    HAL_NVIC_DisableIRQ(DMA1_Stream1_IRQn);
                    HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);
                    HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
                    HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn);

                    HAL_NVIC_DisableIRQ(DMA2D_IRQn);

                    HAL_NVIC_DisableIRQ(EXTI2_IRQn);
                    HAL_NVIC_DisableIRQ(EXTI3_IRQn);
                    HAL_NVIC_DisableIRQ(EXTI4_IRQn);
                    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
                    HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

                    HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
                    HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
                    HAL_NVIC_DisableIRQ(I2C4_EV_IRQn);
                    HAL_NVIC_DisableIRQ(I2C4_ER_IRQn);

                    HAL_NVIC_DisableIRQ(LTDC_IRQn);
                    HAL_NVIC_DisableIRQ(LTDC_ER_IRQn);

                    // HAL_NVIC_DisableIRQ(SPI1_IRQn);
                    HAL_NVIC_DisableIRQ(SPI2_IRQn);
                    HAL_NVIC_DisableIRQ(SPI6_IRQn);

                    HAL_NVIC_DisableIRQ(UART5_IRQn);

                    HAL_NVIC_DisableIRQ(TIM3_IRQn);
                    HAL_NVIC_DisableIRQ(TIM4_IRQn);
                    HAL_NVIC_DisableIRQ(TIM16_IRQn);

                    __enable_irq();

                    HAL_GPIO_WritePin(Layer_RST_GPIO_Port, Layer_RST_Pin, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(Panel_RST_GPIO_Port, Panel_RST_Pin, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(Control_RST_GPIO_Port, Control_RST_Pin, GPIO_PIN_RESET);

                    /////////INIT Hardware
                    if (hspi1.State != HAL_SPI_STATE_READY) {
                        HAL_SPI_Abort(&hspi1); // Abort pending tranmission/receive
                    }
                    HAL_SPI_MspDeInit(&hspi1); // deinit peripherie

                    *((unsigned long *)0x2001FFEC) = 0xBBEEFFFF; // End of RAM
                    HAL_Delay(100);
                    NVIC_SystemReset();
                }
                else if (command.compare("-EEPROM_CLEAR") == 0) {
                    clearEEPROM();
                    println("EEPROM Cleared");
                }
                else if (command.compare("-h") == 0 || command.compare("-help") == 0) {
                    printHelp();
                }
                else if (command.compare("-s") == 0 || command.compare("-status") == 0) {

                    printStatus();
                }
                else if (command.compare("-d") == 0 || command.compare("-device") == 0) {
                    printDeviceManager();
                }
                else if (command.compare("-r") == 0 || command.compare("-restart") == 0) {

                    HAL_Delay(10);
                    HAL_PCD_DevDisconnect(&hpcd_USB_OTG_HS);
                    HAL_Delay(10);
                    HAL_PCD_DevDisconnect(&hpcd_USB_OTG_FS);
                    HAL_Delay(10);
                    USBD_DeInit(&hUsbDeviceHS);
                    HAL_Delay(10);
                    USBD_DeInit(&hUsbDeviceFS);
                    HAL_Delay(10);

                    // USB_CoreReset(hpcd_USB_OTG_FS.Instance);
                    __HAL_RCC_USB_OTG_HS_ULPI_CLK_SLEEP_ENABLE();
                    __HAL_RCC_USB_OTG_FS_ULPI_CLK_SLEEP_ENABLE();
                    __HAL_RCC_USB_OTG_HS_CLK_SLEEP_ENABLE();
                    __HAL_RCC_USB_OTG_FS_CLK_SLEEP_ENABLE();

                    HAL_Delay(10);
                    RCC->AHB1ENR &= ~(RCC_AHB1ENR_USB1OTGHSEN | RCC_AHB1ENR_USB2OTGFSEN);
                    HAL_Delay(500);
                    // USB_CoreReset(hpcd_USB_OTG_FS.Instance);

                    __NVIC_SystemReset();
                }

                else if (command.compare("-getPresetTable") == 0) { // Return Table over COM
                    FlagHandler::COM_PRINT_ENABLE = false;          // disable print
                    sendDataACK();
                    printPresetTable();
                    FlagHandler::COM_PRINT_ENABLE = true; // disable print
                }

                else if (command.compare("-getPresetFromID") == 0) { // Return Preset from ID over COM
                    FlagHandler::COM_PRINT_ENABLE = false;           // disable print

                    sendDataACK();
                    timeout = 0;
                    while (!comAvailable())
                        if (timeout > 500) {
                            println("timeout");
                            data.clear();
                            return;
                        }; // wait for ID

                    uint8_t index = comRead(); // read index

                    if (index > PRESET_NUMBERBLOCKS) {
                        sendDataNACK();
                        return;
                    }

                    sendDataACK();

                    printPreset(index);

                    FlagHandler::COM_PRINT_ENABLE = true; // disable print
                }

                else if (command.compare("-writePresetTable") == 0) { // Write Table over COM
                    FlagHandler::COM_PRINT_ENABLE = false;            // disable print

                    sendDataACK();

                    // Wait for TableBlock
                    if (receiveDataBlock(blockBuffer, TABLE_BLOCKSIZE)) {
                        data.clear();
                        return;
                    }

                    // Check CRC and Marker
                    if (checkBuffer()) {
                        sendDataACK();
                        println("INFO | COM | Write PresetTable..");
                        writePresetTableBlock();
                        updatePresetList();
                        println("INFO | COM |  ..done");
                    }
                    else {
                        sendDataNACK();
                        println("INFO | COM | ChecksumError");
                    }

                    FlagHandler::COM_PRINT_ENABLE = true; // enable print
                }
                else if (command.compare("-writePresetToID") == 0) { // Write Preset over COM to specified ID
                    // Wait for ID
                    FlagHandler::COM_PRINT_ENABLE = false; // disable print

                    sendDataACK();
                    timeout = 0;
                    while (!comAvailable()) {
                        if (timeout > 500) {
                            println("Preset ID -> timeout");
                            data.clear();
                            return;
                        }
                    }

                    uint8_t index = comRead(); // read index

                    if (index > PRESET_NUMBERBLOCKS) {
                        sendDataNACK();
                        return;
                    }

                    sendDataACK();

                    // Wait for PresetBlock
                    if (receiveDataBlock(blockBuffer, PRESET_BLOCKSIZE)) {
                        data.clear();
                        return;
                    }

                    // Check CRC and Marker
                    if (checkBuffer()) {
                        sendDataACK();
                        println("INFO | COM | Write PresetBlock : ", index, "  ..");
                        writePresetBlock(index);
                        println("INFO | COM | ..done");
                    }
                    else {
                        sendDataNACK();
                        println("INFO | COM | ChecksumError");
                    }

                    FlagHandler::COM_PRINT_ENABLE = true; // enable print
                }
                else {
                    println("INFO | COM | Invalid Command.. for help type -h");
                }

                data.clear();

                return; // exit so we dont block the system for too long
            }
        }
    }
}

bool receiveDataBlock(uint8_t *buffer, uint32_t length) {
    elapsedMillis timeout = 0;
    uint32_t chunkIndex = 0;
    for (uint32_t i = 0; i < length; i++) {
        timeout = 0;

        while (!comAvailable()) {
            if (timeout > 500) {
                println("Block -> timeout");
                println("receive: ", i, "   Expected  ", length);
                println("chunkIndex: ", chunkIndex);

                return 1;
            };
        }
        buffer[i] = comRead();

        chunkIndex++;
        if (chunkIndex == CHUNKSIZE || i == (length - 1)) { // reach chunk or block end
            sendDataACK();
            chunkIndex = 0;
        }
    }
    return 0;
}

void supervisor() { // level above the running programm for crashHandling and Debug Communication

    if (FlagHandler::SYS_TIMER > 10000) { // 2seconds no System response -> trigger timeout
        FlagHandler::SYS_TIMEOUT = true;
    }

    if (FlagHandler::SYS_HARDFAULT) {
        shutdown();

        ui.drawSysInfo("SYS_HARDFAULT");
    }

    if (FlagHandler::SYS_TIMEOUT) { // lets wait for external SystemCall
        shutdown();

        ui.drawSysInfo("SYS_TIMEOUT -> wait for COM Input");
        while (true)
            COMmunicateISR();
    }

    if (FlagHandler::SYS_OVERHEAT) { // lets wait for external SystemCall
        shutdown();

        ui.drawSysInfo("SYS_OVERHEAT -> PLEASE COOLDOWN!");
        while (true)
            COMmunicateISR();
    }
}

void shutdown() {
    HAL_GPIO_WritePin(Layer_RST_GPIO_Port, Layer_RST_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Panel_RST_GPIO_Port, Panel_RST_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Control_RST_GPIO_Port, Control_RST_Pin, GPIO_PIN_RESET);

    HAL_NVIC_DisableIRQ(DMA1_Stream0_IRQn);
    HAL_NVIC_DisableIRQ(DMA1_Stream1_IRQn);
    HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);
    HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
    HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn);

    HAL_NVIC_DisableIRQ(DMA2D_IRQn);

    HAL_NVIC_DisableIRQ(EXTI2_IRQn);
    HAL_NVIC_DisableIRQ(EXTI3_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_IRQn);
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

    HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
    HAL_NVIC_DisableIRQ(I2C4_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C4_ER_IRQn);

    HAL_NVIC_DisableIRQ(SPI1_IRQn);
    HAL_NVIC_DisableIRQ(SPI2_IRQn);
    HAL_NVIC_DisableIRQ(SPI6_IRQn);

    HAL_NVIC_DisableIRQ(UART5_IRQn);

    HAL_NVIC_DisableIRQ(TIM3_IRQn);
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
    HAL_NVIC_DisableIRQ(TIM16_IRQn);
}

#endif