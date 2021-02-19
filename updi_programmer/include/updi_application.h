#ifndef __UPDI_APPLICATION_H__
#define __UPDI_APPLICATION_H__

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "device.h"
#include "updi_instruction_set.h"

namespace updi {

/*
 * @brief The UpdiApplication class
 *
 * This class provides APIs for flashing, reading or easing the chip.
 * Basically, each API is a combination of UDPI instruction sets.
 * Interfaces will be invoked by @ref NvmProgrammer
 */
class UpdiApplication {
   public:
    UpdiApplication(const std::string&                port,
                    uint32_t                          baud_rate,
                    const std::shared_ptr<AvrDevice>& device);
    ~UpdiApplication();

    /*
     * @brief read sib and check PDI version
     *
     * Different chip may reuqire different operation steps.
     * We should read sib and check NVM interface to decide
     * which PDI version we should obey with.
     *
     * Note:
     *    It may throw @ref UpdiException if UPDI interface is not ready yet.
     *
     * @return SIB information
     */
    std::string init_nvm_operation();

    /*
     * @brief check if chip is already in programming mode
     *
     * @return true/false
     */
    bool in_prog_mode();

    /*
     * @brief unlock chip
     *
     * - Write chip erase key first,
     * - Check @ref UPDI_ASI_SYS_STATUS register to make sure lock bit is
     * cleared.
     * - Write NVMPORG key as well.
     * - Toggle reset
     * - Wait for chip unlocked
     *
     * Note:
     *    It may throw @ref UpdiException if it fails to unlock the chip.
     */
    void unlock();

    /*
     * @brief enter into NVM programming mode
     *
     * Note:
     *    It may throw @ref UpdiException if it fails to enter programming mode.
     */
    void enter_progmode();

    /*
     * @brief disable UPDI
     *
     * All UPDI PHY confugration and keys will be reset
     */
    void leave_progmode();

    /*
     * @brief apply or release an UPDI reset condition
     *
     * Programmer should call reset(true)/reset(false) to simulate a reset
     * action.
     *
     * Note:
     *    It may throw @ref UpdiException if it fails to reset UPDI.
     */
    void reset(bool apply_reset);

    /*
     * @brief erase chip
     *
     * Note:
     *    It may throw @ref UpdiException if it fails to erase chip.
     */
    void chip_erase();

    /*
     * @brief write a NVM page
     *
     * Note:
     *    It may throw @ref UpdiException if it fails to write page buffer.
     *
     * @param[in] start_addr NVM page start address
     * @param[in] page_data page data to be written

     */
    void write_nvm_page(uint32_t                    start_addr,
                        const std::vector<uint8_t>& page_data);

    /*
     * @brief write a number of bytes to memory
     *
     * Note:
     *    It may throw @ref UpdiException if data size is out of limits.
     *
     * @param[in] start_addr location where data should be written
     * @param[in] data a byte array to write
     */
    void write_data(uint32_t address, const std::vector<uint8_t>& data);

    /*
     * @brief write a number of words to memory
     *
     * Note:
     *    It may throw @ref UpdiException if data size is out of limits.
     *
     * @param[in] start_addr location where data should be written
     * @param[in] data an byte array to write
     */
    void write_data_words(uint32_t address, const std::vector<uint8_t>& data);

    /*
     * @brief read a number of bytes from UDPI
     *
     * Note:
     *    It may throw @ref UpdiException if byte size is out of limits.
     *
     * @param[in] address location where data should be read from
     * @param[in] byte_size number of bytes to read
     *
     * @return byte array from specified address
     */
    std::vector<uint8_t> read_data(uint32_t address, uint32_t byte_size);

    /*
     * @brief read a number of words from UDPI
     *
     * Note:
     *    It may throw @ref UpdiException if byte size is out of limits.
     *
     * @param[in] address location where data should be read from
     * @param[in] word_size number of words to read
     *
     * @return byte array from specified address
     */
    std::vector<uint8_t> read_data_words(uint32_t address, uint32_t word_size);

    /*
     * @brief write specified fuse data
     *
     * Note:
     *    It may throw @ref UpdiException if PDI version is 2 or chip is not in
     * programming mode.
     *
     * @param[in] fuse_number fuse offset number e.g., 0x01 BODCFG
     * @param[in] value fuse value to update
     */
    void write_fuse_data(uint32_t fuse_number, uint8_t value);

    /*
     * @brief write specified fuse data
     *
     * @param[in] fuse_number fuse offset number e.g., 0x01 BODCFG
     * @return fuse value
     */
    uint8_t read_fuse_data(uint32_t fuse_number);

   private:
    bool wait_unlocked(uint32_t timeout_ms);
    void write_progmode_key();
    bool wait_flash_ready();
    void execute_nvm_command(uint8_t command);

    std::unique_ptr<UpdiInstruction> _updi_instruction;
    std::shared_ptr<AvrDevice>       _avr_device;
    bool                             _pdi_v2;
};

}  // namespace updi

#endif