#ifndef __AVR_DEVICE_H__
#define __AVR_DEVICE_H__

#include <stdint.h>

#include <iostream>
#include <regex>
#include <set>
#include <sstream>
#include <string>

#define DEFAULT_SYSCFG_ADDRESS 0x0F00
#define DEFAULT_NVMCTRL_ADDRESS 0x1000
#define DEFAULT_SIGROW_ADDRESS 0x1100
#define DEFAULT_FUSES_ADDRESS 0x1280
#define DEFAULT_USERROW_ADDRESS 0x1300

namespace updi {
// avr Dx series
static std::set<std::string> avr_d_series = {
    "avr128da28", "avr128da32", "avr128da48", "avr128da64", "avr64da28",
    "avr64da32",  "avr64da48",  "avr64da64",  "avr32da28",  "avr32da32",
    "avr32da48",  "avr128db28", "avr128db32", "avr128db48", "avr128db64",
    "avr64db28",  "avr64db32",  "avr64db48",  "avr64db64",  "avr32db28",
    "avr32db32",  "avr32db48",  "avr64dd14",  "avr64dd20",  "avr64dd28",
    "avr64dd32",  "avr32dd14",  "avr32dd20",  "avr32dd28",  "avr32dd32",
    "avr16dd14",  "avr16dd20",  "avr16dd28",  "avr16dd32"};

// mega AVR series
static std::set<std::string> avr_mega_48k = {"mega4808", "mega4809"};
static std::set<std::string> avr_mega_32k = {"mega3208", "mega3209"};
static std::set<std::string> avr_mega_16k = {"mega1608", "mega1609"};
static std::set<std::string> avr_mega_8k = {"mega808", "mega809"};

// tiny avr series
static std::set<std::string> tiny_32k = {"tiny3216", "tiny3217"};
static std::set<std::string> tiny_16k = {"tiny1604", "tiny1606", "tiny1607",
                                  "tiny1614", "tiny1616", "tiny1617"};
static std::set<std::string> tiny_8k = {"tiny804", "tiny806", "tiny807",
                                 "tiny814", "tiny816", "tiny817"};
static std::set<std::string> tiny_4k = {"tiny402", "tiny404", "tiny406", "tiny412",
                                 "tiny414", "tiny416", "tiny417"};
static std::set<std::string> tiny_2k = {"tiny202", "tiny204", "tiny212", "tiny214"};

/*
 * @brief The AvrDevice class
 *
 * This class is used for setting up memory map based on chip model.
 *
 * The full supported device list can be found through @ref
 * get_supported_devices.
 */
class AvrDevice {
   public:
    AvrDevice(const std::string& device_name)
        : name(device_name),
          syscfg_base_addr(DEFAULT_SYSCFG_ADDRESS),
          nvmctrl_base_addr(DEFAULT_NVMCTRL_ADDRESS),
          sigrow_base_addr(DEFAULT_SIGROW_ADDRESS),
          fuses_base_addr(DEFAULT_FUSES_ADDRESS),
          userrow_base_addr(DEFAULT_USERROW_ADDRESS) {
        lock_address = 0;

        if (avr_d_series.find(device_name) != avr_d_series.end()) {
            fuses_base_addr = 0x1050;
            userrow_base_addr = 0x1080;
            lock_address = 0x1040;
            flash_start_addr = 0x800000;
            flash_page_size = 256;

            std::regex  r("\\d+");
            std::smatch sm;
            if (std::regex_search(device_name, sm, r)) {
                flash_size = std::stoi(sm.str()) * 1024;
            }
        } else if (avr_mega_48k.find(device_name) != avr_mega_48k.end()) {
            flash_start_addr = 0x4000;
            flash_size = 48 * 1024;
            flash_page_size = 128;
        } else if (avr_mega_32k.find(device_name) != avr_mega_32k.end()) {
            flash_start_addr = 0x4000;
            flash_size = 32 * 1024;
            flash_page_size = 128;
        } else if (avr_mega_16k.find(device_name) != avr_mega_16k.end()) {
            flash_start_addr = 0x4000;
            flash_size = 16 * 1024;
            flash_page_size = 64;
        } else if (avr_mega_8k.find(device_name) != avr_mega_8k.end()) {
            flash_start_addr = 0x4000;
            flash_size = 8 * 1024;
            flash_page_size = 64;
        } else if (tiny_32k.find(device_name) != tiny_32k.end()) {
            flash_start_addr = 0x8000;
            flash_size = 32 * 1024;
            flash_page_size = 128;
        } else if (tiny_16k.find(device_name) != tiny_16k.end()) {
            flash_start_addr = 0x8000;
            flash_size = 16 * 1024;
            flash_page_size = 64;
        } else if (tiny_8k.find(device_name) != tiny_8k.end()) {
            flash_start_addr = 0x8000;
            flash_size = 8 * 1024;
            flash_page_size = 64;
        } else if (tiny_4k.find(device_name) != tiny_4k.end()) {
            flash_start_addr = 0x8000;
            flash_size = 4 * 1024;
            flash_page_size = 64;
        } else if (tiny_2k.find(device_name) != tiny_2k.end()) {
            flash_start_addr = 0x8000;
            flash_size = 2 * 1024;
            flash_page_size = 64;
        } else {
            /* Unsupported device*/
            std::cerr << "Unknown device" << std::endl;
        }
    }

    ~AvrDevice() {
    }

    /*
     * @brief get the base address to read the revision ID
     * @return SYSCFG base address
     */
    uint32_t get_syscfg_addr() {
        return syscfg_base_addr;
    }

    /*
     * @brief get the base address to Nonvolatile Memory Controller
     * @return NVMCTRL base address
     */
    uint32_t get_nvmctrl_addr() {
        return nvmctrl_base_addr;
    }

    /*
     * @brief get the base address to Signature Row
     * @return SIGROW base address
     */
    uint32_t get_sigrow_addr() {
        return sigrow_base_addr;
    }

    /*
     * @brief get the base address to Device-specific fuses
     * @return FUSES base address
     */
    uint32_t get_fuses_addr() {
        return fuses_base_addr;
    }

    /*
     * @brief get the base address to User Row
     * @return USERROW base address
     */
    uint32_t get_userrow_addr() {
        return userrow_base_addr;
    }

    /*
     * @brief get the lock address
     *        not apply to avr-tiny series
     * @return LOCK register address
     */
    uint32_t get_lock_addr() {
        return lock_address;
    }

    /*
     * @brief get the start address of flash code
     * @return base address of flash code
     */
    uint32_t get_flash_start_addr() {
        return flash_start_addr;
    }

    /*
     * @brief get the total flash size
     * @return flash size of the device
     */
    uint32_t get_flash_size() {
        return flash_size;
    }

    /*
     * @brief get the page size for read/write
     * @return page buffer size
     */
    uint32_t get_flash_pagesize() {
        return flash_page_size;
    }

    /*
     * @brief get the supported device list
     * @return all supported device models
     */
    static std::string get_supported_devices() {
        std::set<std::string> all_devices;
        all_devices.insert(avr_d_series.begin(), avr_d_series.end());
        all_devices.insert(avr_mega_48k.begin(), avr_mega_48k.end());
        all_devices.insert(avr_mega_32k.begin(), avr_mega_32k.end());
        all_devices.insert(avr_mega_16k.begin(), avr_mega_16k.end());
        all_devices.insert(avr_mega_8k.begin(), avr_mega_8k.end());

        all_devices.insert(tiny_32k.begin(), tiny_32k.end());
        all_devices.insert(tiny_16k.begin(), tiny_16k.end());
        all_devices.insert(tiny_8k.begin(), tiny_8k.end());
        all_devices.insert(tiny_4k.begin(), tiny_4k.end());
        all_devices.insert(tiny_2k.begin(), tiny_2k.end());

        std::stringstream ss;
        int count = 0;

        ss << "\r\n";
        for (auto& d : all_devices) {
            ss << d << ' ';
            count++;

            if (count % 6 == 0) {
                ss << "\r\n";
            }
        }

        return ss.str();
    }

   private:
    std::string name;
    uint32_t    syscfg_base_addr;
    uint32_t    nvmctrl_base_addr;
    uint32_t    sigrow_base_addr;
    uint32_t    fuses_base_addr;
    uint32_t    userrow_base_addr;

    uint32_t lock_address;
    uint32_t flash_start_addr;
    uint32_t flash_size;
    uint32_t flash_page_size;
};

}  // namespace updi

#endif