#ifndef __UDPI_COMMON_CONSTANTS_H__
#define __UDPI_COMMON_CONSTANTS_H__

#include <stdint.h>

#include <exception>
#include <string>

namespace updi {

#define UPDI_NVM_STATUS_WRITE_ERROR 2
#define UPDI_NVM_STATUS_EEPROM_BUSY 1
#define UPDI_NVM_STATUS_FLASH_BUSY 0

constexpr uint8_t UPDI_BREAK = 0x00;

constexpr uint8_t UPDI_LDS = 0x00;
constexpr uint8_t UPDI_STS = 0x40;
constexpr uint8_t UPDI_LD = 0x20;
constexpr uint8_t UPDI_ST = 0x60;
constexpr uint8_t UPDI_LDCS = 0x80;
constexpr uint8_t UPDI_STCS = 0xC0;
constexpr uint8_t UPDI_REPEAT = 0xA0;
constexpr uint8_t UPDI_KEY = 0xE0;

constexpr uint8_t UPDI_PTR = 0x00;
constexpr uint8_t UPDI_PTR_INC = 0x04;
constexpr uint8_t UPDI_PTR_ADDRESS = 0x08;

constexpr uint8_t UPDI_ADDRESS_8 = 0x00;
constexpr uint8_t UPDI_ADDRESS_16 = 0x04;
constexpr uint8_t UPDI_ADDRESS_24 = 0x08;

constexpr uint8_t UPDI_DATA_8 = 0x00;
constexpr uint8_t UPDI_DATA_16 = 0x01;
constexpr uint8_t UPDI_DATA_24 = 0x02;

constexpr uint8_t UPDI_KEY_SIB = 0x04;
constexpr uint8_t UPDI_KEY_KEY = 0x00;

constexpr uint8_t UPDI_KEY_64 = 0x00;
constexpr uint8_t UPDI_KEY_128 = 0x01;

constexpr uint8_t UPDI_SIB_8BYTES = UPDI_KEY_64;
constexpr uint8_t UPDI_SIB_16BYTES = UPDI_KEY_128;

constexpr uint8_t UPDI_REPEAT_BYTE = 0x00;
constexpr uint8_t UPDI_REPEAT_WORD = 0x01;

constexpr uint8_t UPDI_PHY_SYNC = 0x55;
constexpr uint8_t UPDI_PHY_ACK = 0x40;

// Repeat counter of 1-byte, with off-by-one counting
constexpr uint32_t UPDI_MAX_REPEAT_SIZE = (0xFF + 1);

// CS and ASI Register Address map
constexpr uint8_t UPDI_CS_STATUSA = 0x00;
constexpr uint8_t UPDI_CS_STATUSB = 0x01;
constexpr uint8_t UPDI_CS_CTRLA = 0x02;
constexpr uint8_t UPDI_CS_CTRLB = 0x03;
constexpr uint8_t UPDI_ASI_KEY_STATUS = 0x07;
constexpr uint8_t UPDI_ASI_RESET_REQ = 0x08;
constexpr uint8_t UPDI_ASI_CTRLA = 0x09;
constexpr uint8_t UPDI_ASI_SYS_CTRLA = 0x0A;
constexpr uint8_t UPDI_ASI_SYS_STATUS = 0x0B;
constexpr uint8_t UPDI_ASI_CRC_STATUS = 0x0C;

constexpr uint8_t UPDI_CTRLA_IBDLY_BIT = 7;
constexpr uint8_t UPDI_CTRLA_RSD_BIT = 3;
constexpr uint8_t UPDI_CTRLB_CCDETDIS_BIT = 3;
constexpr uint8_t UPDI_CTRLB_UPDIDIS_BIT = 2;

const std::string UPDI_KEY_NVM = "NVMProg ";
const std::string UPDI_KEY_CHIPERASE = "NVMErase";

constexpr uint8_t UPDI_ASI_STATUSA_REVID = 4;
constexpr uint8_t UPDI_ASI_STATUSB_PESIG = 0;

constexpr uint8_t UPDI_ASI_KEY_STATUS_CHIPERASE = 3;
constexpr uint8_t UPDI_ASI_KEY_STATUS_NVMPROG = 4;
constexpr uint8_t UPDI_ASI_KEY_STATUS_UROWWRITE = 5;

constexpr uint8_t UPDI_ASI_SYS_STATUS_RSTSYS = 5;
constexpr uint8_t UPDI_ASI_SYS_STATUS_INSLEEP = 4;
constexpr uint8_t UPDI_ASI_SYS_STATUS_NVMPROG = 3;
constexpr uint8_t UPDI_ASI_SYS_STATUS_UROWPROG = 2;
constexpr uint8_t UPDI_ASI_SYS_STATUS_LOCKSTATUS = 0;

constexpr uint8_t UPDI_RESET_REQ_VALUE = 0x59;

// NVMCTRL register map
constexpr uint8_t UPDI_NVMCTRL_CTRLA = 0x00;
constexpr uint8_t UPDI_NVMCTRL_CTRLB = 0x01;
constexpr uint8_t UPDI_NVMCTRL_STATUS = 0x02;
constexpr uint8_t UPDI_NVMCTRL_INTCTRL = 0x03;
constexpr uint8_t UPDI_NVMCTRL_INTFLAGS = 0x04;
constexpr uint8_t UPDI_NVMCTRL_DATAL = 0x06;
constexpr uint8_t UPDI_NVMCTRL_DATAH = 0x07;
constexpr uint8_t UPDI_NVMCTRL_ADDRL = 0x08;
constexpr uint8_t UPDI_NVMCTRL_ADDRH = 0x09;

// NVMCTRL v0 CTRLA commands
constexpr uint8_t UPDI_V0_NVMCTRL_CTRLA_NOP = 0x00;
constexpr uint8_t UPDI_V0_NVMCTRL_CTRLA_WRITE_PAGE = 0x01;
constexpr uint8_t UPDI_V0_NVMCTRL_CTRLA_ERASE_PAGE = 0x02;
constexpr uint8_t UPDI_V0_NVMCTRL_CTRLA_ERASE_WRITE_PAGE = 0x03;
constexpr uint8_t UPDI_V0_NVMCTRL_CTRLA_PAGE_BUFFER_CLR = 0x04;
constexpr uint8_t UPDI_V0_NVMCTRL_CTRLA_CHIP_ERASE = 0x05;
constexpr uint8_t UPDI_V0_NVMCTRL_CTRLA_ERASE_EEPROM = 0x06;
constexpr uint8_t UPDI_V0_NVMCTRL_CTRLA_WRITE_FUSE = 0x07;

// NVMCTRL v1 CTRLA commands
constexpr uint8_t UPDI_V1_NVMCTRL_CTRLA_NOCMD = 0x00;
constexpr uint8_t UPDI_V1_NVMCTRL_CTRLA_FLASH_WRITE = 0x02;
constexpr uint8_t UPDI_V1_NVMCTRL_CTRLA_EEPROM_ERASE_WRITE = 0x13;
constexpr uint8_t UPDI_V1_NVMCTRL_CTRLA_CHIP_ERASE = 0x20;

/**
 * @brief An Exception type thrown by UPDI programmer
 */
struct UpdiException : public std::runtime_error {
    /**
     * @brief An Exception type thrown by UPDI programmer.
     * @param[in] message The message associated with the exception.
     */
    UpdiException(const std::string& message) : std::runtime_error(message) {
    }
};
}  // namespace updi

#endif