#ifndef __INTEL_HEX_FILE_H__
#define __INTEL_HEX_FILE_H__

#include <stdint.h>

#include <string>
#include <vector>

namespace updi {

struct ProgramPage {
    size_t               address;
    size_t               pageSize;
    std::vector<uint8_t> data;
};

/*
 * @brief The IntelHexFile class
 *
 * This class is used for parsing Intel Hex formatted file and
 * arrange firmware data into pages (padding the data so it will align with page
 * size).
 *
 * The splitted page data will be requested by @ref NvmProgrammer.
 * Note:
 *     Only record type 0 and 1 are supported.
 */
class IntelHexFile {
   public:
    IntelHexFile(uint32_t flash_size, uint32_t page_size);
    ~IntelHexFile();

    /*
     * @brief load Atmel studio generated hex file
     *
     * @param[in] filename full path of the hex file
     * @return flash start address
     */
    int load_file(const std::string& filename);

    /*
     * @brief get the splitted and padded pages
     *
     * @return multiple page data for flashing
     */
    const std::vector<ProgramPage>& get_page_data();

    /*
     * @brief get the original binary data of the firmware
     *
     * @return whole firmware data (without any padding)
     */
    const std::vector<uint8_t>& get_flash_data();

   private:
    int parse_record(const std::string& record, std::string& error);

    uint32_t                 nvm_flash_size;
    uint32_t                 nvm_page_size;
    uint32_t                 firmware_size;
    std::vector<uint8_t>     nvm_data;
    std::vector<ProgramPage> nvm_pages;
};

}  // namespace updi

#endif
