#ifndef __UPDI_INSTRUCTION_SET_H__
#define __UPDI_INSTRUCTION_SET_H__

#include <stdint.h>

#include <memory>
#include <string>

#include "updi_serial.h"

namespace updi {

/*
 * @brief The UpdiInstruction class
 *
 * This class handles the UPDI data protocol within the device.
 * It provides support for UDPI instruction sets.
 * Interfaces will be invoked by @ref UpdiApplication
 */
class UpdiInstruction {
   public:
    UpdiInstruction(const std::string& port, uint32_t baud_rate);
    ~UpdiInstruction();

    /*
     * @brief load data from Control/Status register
     *
     * @return Control/Status register value
     */
    uint8_t ldcs(uint8_t reg_addr);

    /*
     * @brief store a vlaue to Control/Status register
     */
    void stcs(uint8_t reg_address, uint8_t value);

    /*
     * @brief load a single byte directly from 16/24-bit address
     *
     * Note:
     *     It may throw @ref UpdiException if reading a byte fails
     *
     * @return byte value at the specified address
     */
    uint8_t ld(uint32_t address);

    /*
     * @brief load a word directly from 16/24-bit address
     *
     * @return word value at the specified address
     */
    std::vector<uint8_t> ld16(uint32_t address);

    /*
     * @brief store a byte directly to a 16/24-bit address
     *
     * Note:
     *     It may throw @ref UpdiException if storing a byte fails
     */
    void st(uint32_t address, uint8_t value);

    /*
     * @brief store a word directly to a 16/24-bit address
     *
     * Note:
     *     It may throw @ref UpdiException if storing a word fails
     */
    void st16(uint32_t address, uint16_t value);

    /*
     * @brief load a number of bytes from the pointer location with pointer
     * post-inc
     *
     * Before it is called, @ref st_ptr should be called to set up the pointer
     * address.
     *
     * @param[in] size bytes to load
     * @return an byte array
     */
    std::vector<uint8_t> ld_ptr_inc(uint32_t size);

    /*
     * @brief load a number of bytes from the pointer location with pointer
     * post-inc
     *
     * Before it is called, @ref st_ptr should be called to set up the pointer
     * address.
     *
     * @param[in] size words to load
     * @return an byte array
     */
    std::vector<uint8_t> ld_ptr_inc16(uint32_t size);

    /*
     * @brief set the pointer address location
     *
     * Note:
     *     It may throw @ref UpdiException if pointer location setup fails.
     *
     * @param[in] address pointer address (16/24 bits)
     */
    void st_ptr(uint32_t address);

    /*
     * @brief store a number of bytes to the pointer location with pointer
     * post-inc
     *
     * Note:
     *     It may throw @ref UpdiException if pointer location with post-inc
     * setup fails.
     *
     * @param[in] data byte array to store
     */
    void st_ptr_inc(const std::vector<uint8_t>& data);

    /*
     * @brief store a number of words to the pointer location with pointer
     * post-inc
     *
     * @param[in] data byte array to store
     */
    void st_ptr_inc16(const std::vector<uint8_t>& data);

    /*
     * @brief store a value to the repeat counter
     *
     * This is useful to avoid SYNCH overhead and instruction frame.
     * Memory instructions can read/write data continuously.
     *
     * @param[in] repeats repeat size
     */
    void repeat(uint32_t repeats);

    /*
     * @brief read system information block
     *
     * @return SIB data in ASCII format
     */
    std::string read_sib();

    /*
     * @brief write key to activate protected features
     *
     *        e.g., erase chip or program NVM
     *
     * @param[in] key specified feature key
     */
    void key(const std::string& key);

    /*
     * @brief enable or disable 24bit address support
     *
     * 24 bit address space is only available if NVM interface is PDI V2.
     */
    void enable_24bit_address(bool mode) {
        _use_24bit_addr = mode;
    }

    /*
     * @brief check if communication with UPDI is good
     *
     * @return true if updi instruction gets response; Otherwise, false
     */
    bool updi_is_ready();

   private:
    void init();

    std::unique_ptr<UpdiSerial> _serial_comm;
    bool                        _use_24bit_addr;
};

}  // namespace updi

#endif