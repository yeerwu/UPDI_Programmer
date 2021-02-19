#ifndef __NVM_PROGRAMMER_H__
#define __NVM_PROGRAMMER_H__

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "device.h"
#include "intel_hexfile.h"
#include "updi_application.h"

namespace updi {

/*
 * @brief The NvmProgrammer class
 *
 * This class provides high level APIs for application level logic
 * - retrieve the device sib details
 * - enter/leave programming mode
 * - erase chip
 * - unlock device so device will be prepared for erasing/programming
 * - write flash and read flash data for verification
 * - read and write fuses (with specified offset)
 *
 * Application should follow below steps to flash the device
 * - get_device_info
 * - enter_progmode
 * - chip_erase
 * - write_flash
 * - read_flash
 * - leave_progmode
 */
class NvmProgrammer {
   public:
    NvmProgrammer(const std::string& port,
                  uint32_t           baud_rate,
                  const std::string& device_name);
    ~NvmProgrammer();

    /*
     * @brief get device SIB details
     *
     * @return SIB string description
     */
    std::string get_device_info();

    /*
     * @brief enter programming mode
     *
     * Note:
     *    It may throw @ref UpdiException if NVM fails to enter programming mode. 
     */
    void enter_progmode();

    /*
     * @brief leave programming mode
     *
     */
    void leave_progmode();

    /*
     * @brief unlock device if @ref enter_progmode throws exception
     *
     * It may thrown exception @ref UpdiException if unlock fails.
     */
    void unlock_device();

    /*
     * @brief erase chip
     *
     * It may thrown exception @ref UpdiException if chip is not in programming
     * mode or erase fails.
     */
    void chip_erase();

    /*
     * @brief read a number of pages from a base address
     *
     * It may thrown exception @ref UpdiException if chip is not in programming
     * mode.
     * 
     * @param[in] address start address to read from
     * @param[in] size number of bytes to read (multiple of page sizes)
     *
     * @return multiple page data read from flash
     */
    std::vector<uint8_t> read_flash(uint32_t address, uint32_t size);

    /*
     * @brief write a number of pages from a base address
     *
     * It may thrown exception @ref UpdiException if chip is not in programming
     * mode.
     * 
     * @param[in] address base offset to write to
     * @param[in] pages pages of data to write
     *
     */
    void write_flash(uint32_t address, const std::vector<ProgramPage>& pages);

    /*
     * @brief read specified fuse value
     *
     * It may thrown exception @ref UpdiException if chip is not in programming
     * mode.
     * 
     * For tiny416, the fuse nubmer is as below:
     * -0x00: WDTCFG
     * -0x01: BODCFG
     * -0x02: OSCCFG
     * -0x04: TCD0CFG
     * -0x05: SYSCFG0
     * -0x06: SYSCFG1
     * -0x07: APPEND
     * -0x08: BOOTEND
     * -0x0A: LOCKBIT
     * 
     * @param[in] fuse_num fuse offset
     *
     * @return fuse value
     */
    uint8_t read_fuse(uint32_t fuse_num);

    /*
     * @brief write specified fuse value
     * 
     * It may thrown exception @ref UpdiException if chip is not in programming
     * mode.
     * 
     * For tiny416, the fuse nubmer is as below:
     * -0x00: WDTCFG
     * -0x01: BODCFG
     * -0x02: OSCCFG
     * -0x04: TCD0CFG
     * -0x05: SYSCFG0
     * -0x06: SYSCFG1
     * -0x07: APPEND
     * -0x08: BOOTEND
     * -0x0A: LOCKBIT
     * 
     * @param[in] fuse_num fuse offset
     * @param[in] value fuse value to update
     *
     */
    void write_fuse(uint32_t fuse_num, uint8_t value);

    /*
     * @brief get the shared @ref AvrDevice
     *
     * @return a shared @ref AvrDevice object
     */
    std::shared_ptr<AvrDevice> get_device() const {
        return _avr_device;
    }

   private:
    std::shared_ptr<AvrDevice>       _avr_device;
    std::unique_ptr<UpdiApplication> _updi_application;
    bool                             _programming;
};

}  // namespace updi

#endif