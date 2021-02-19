#include "updi_application.h"

#include <chrono>
#include <iostream>

#include <unistd.h>

#include "updi_common.h"

using namespace std;
using namespace chrono;

namespace updi {

UpdiApplication::UpdiApplication(const string&                port,
                                 uint32_t                     baud_rate,
                                 const shared_ptr<AvrDevice>& device)
    : _avr_device(device), _pdi_v2(false) {
    _updi_instruction = make_unique<UpdiInstruction>(port, baud_rate);
}

UpdiApplication::~UpdiApplication() {
}

string UpdiApplication::init_nvm_operation() {
    if (!_updi_instruction->updi_is_ready()) {
        throw UpdiException("Updi interface is not ready yet");
    }

    string sib = _updi_instruction->read_sib();
    if (sib.size() < 16) {
        cerr << "Failed to reading sib details" << endl;
        return "";
    }

    // Check NVM interface version
    string nvm = sib.substr(8, 3);
    if (nvm == "P:2") {
        cout << "Using PDI v2 interfaces" << endl;
        _pdi_v2 = true;

        // PDI V2 uses 24bit address instaed of 16bit
        _updi_instruction->enable_24bit_address(true);
    }

    string ocd = sib.substr(11, 3);
    cout << "Debug interface: " << ocd << endl;

    return sib;
}

bool UpdiApplication::in_prog_mode() {
    uint8_t asi_status = _updi_instruction->ldcs(UPDI_ASI_SYS_STATUS);
    return (asi_status & (1 << UPDI_ASI_SYS_STATUS_NVMPROG)) != 0;
}

void UpdiApplication::unlock() {
    _updi_instruction->key(UPDI_KEY_CHIPERASE);
    uint8_t key_status = _updi_instruction->ldcs(UPDI_ASI_KEY_STATUS);
    key_status &= (1 << UPDI_ASI_KEY_STATUS_CHIPERASE);

    if (!key_status) {
        throw UpdiException("CHIPERASE key is not accepted");
    }

    // Insert NVMProg key as well
    // In case of CRC being enabled, the chip must be left in programming mode
    // after the erase
    write_progmode_key();

    // Toggle reset
    reset(true);
    reset(false);

    // Wait for chip unlock
    if (!wait_unlocked(100)) {
        throw UpdiException("Faield to erase chip using key");
    }
}

void UpdiApplication::enter_progmode() {
    write_progmode_key();

    // Toggle reset
    reset(true);
    reset(false);

    cout << "Wait for NVMGPROG status" << endl;

    // Timeout 1s
    auto start = system_clock::now();
    while (1) {
        uint8_t key_status = _updi_instruction->ldcs(UPDI_ASI_KEY_STATUS);
        key_status &= (1 << UPDI_ASI_KEY_STATUS_NVMPROG);
        if (key_status) {
            break;
        }

        auto duration =
            duration_cast<milliseconds>(system_clock::now() - start).count();
        if (duration > 1000) {
            break;
        }
        usleep(10 * 1000);
    }

    if (!in_prog_mode()) {
        throw UpdiException("Failed to enter NVM programming mode");
    }
}

void UpdiApplication::leave_progmode() {
    cout << "Leaving NVM programming mode" << endl;

    // Toggle reset
    reset(true);
    reset(false);

    // Disable UPDI
    _updi_instruction->stcs(UPDI_CS_CTRLB, (1 << UPDI_CTRLB_UPDIDIS_BIT) |
                                               (1 << UPDI_CTRLB_CCDETDIS_BIT));
}

void UpdiApplication::reset(bool apply_reset) {
    if (apply_reset) {
        cout << "Apply UPDI reset" << endl;
        _updi_instruction->stcs(UPDI_ASI_RESET_REQ, UPDI_RESET_REQ_VALUE);

        uint8_t sys_status = _updi_instruction->ldcs(UPDI_ASI_SYS_STATUS);
        sys_status &= (1 << UPDI_ASI_SYS_STATUS_RSTSYS);
        if (!sys_status) {
            throw UpdiException("Error applying reset");
        }
    } else {
        cout << "Release UPDI reset" << endl;
        _updi_instruction->stcs(UPDI_ASI_RESET_REQ, 0);

        // Wait for reset to complete
        // Timeout 500ms
        auto start = system_clock::now();
        while (1) {
            uint8_t sys_status = _updi_instruction->ldcs(UPDI_ASI_SYS_STATUS);
            sys_status &= (1 << UPDI_ASI_SYS_STATUS_RSTSYS);
            if (!sys_status) {
                break;
            }

            auto duration =
                duration_cast<milliseconds>(system_clock::now() - start)
                    .count();
            if (duration > 500) {
                throw UpdiException("Still active reset status");
            }
            usleep(10 * 1000);
        }
    }
}

void UpdiApplication::chip_erase() {
    if (_pdi_v2) {
        throw UpdiException("PDI V2 is not supported now");
    }

    if (!wait_flash_ready()) {
        throw UpdiException("Waiting for flash ready timed out");
    }

    execute_nvm_command(UPDI_V0_NVMCTRL_CTRLA_CHIP_ERASE);

    // Wait for erasing to complete
    if (!wait_flash_ready()) {
        throw UpdiException("Waiting for flash ready after erase timed out");
    }
}

void UpdiApplication::write_nvm_page(uint32_t               start_addr,
                                     const vector<uint8_t>& page_data) {
    if (_pdi_v2) {
        throw UpdiException("PDI V2 is not supported now");
    }

    if (!wait_flash_ready()) {
        throw UpdiException("Waiting for flash ready timed out");
    }

    cout << "Clear page buffer" << endl;
    execute_nvm_command(UPDI_V0_NVMCTRL_CTRLA_PAGE_BUFFER_CLR);

    if (!wait_flash_ready()) {
        throw UpdiException(
            "Waiting for flash ready after page buffer clear timed out");
    }

    // write page data to page buffer
    write_data_words(start_addr, page_data);

    // write page buffer data to NVM
    execute_nvm_command(UPDI_V0_NVMCTRL_CTRLA_WRITE_PAGE);

    if (!wait_flash_ready()) {
        throw UpdiException(
            "Waiting for flash ready after page write timed out");
    }
}

void UpdiApplication::write_data(uint32_t               address,
                                 const vector<uint8_t>& data) {
    // special case for only writing 1 byte
    if (data.size() == 1) {
        _updi_instruction->st(address, data[0]);
        return;
    }

    // special case for only writing 2 bytes
    if (data.size() == 2) {
        _updi_instruction->st(address, data[0]);
        _updi_instruction->st(address + 1, data[1]);
        return;
    }

    // if writing more than 2 bytes, then repeat command is needed
    if (data.size() > UPDI_MAX_REPEAT_SIZE) {
        throw UpdiException("Data size exceeds the limits");
    }

    _updi_instruction->st_ptr(address);

    // Repeat to write the byte array
    _updi_instruction->repeat(data.size());
    _updi_instruction->st_ptr_inc(data);
}

void UpdiApplication::write_data_words(uint32_t               address,
                                       const vector<uint8_t>& data) {
    // special case for only writing 1 word
    if (data.size() == 2) {
        uint16_t value = ((uint16_t)data[1] << 8) + data[0];
        _updi_instruction->st16(address, value);
        return;
    }

    if ((data.size() % 2) != 0 ) {
        throw UpdiException("Data size should align on word width");
    }
    
    // for repeated word operation, the maximum bytes should be MAX_REPEAT_SIZE
    // *2
    if (data.size() > (UPDI_MAX_REPEAT_SIZE * 2)) {
        throw UpdiException("Data size exceeds the limits");
    }

    _updi_instruction->st_ptr(address);

    // Repeat to write the byte array
    _updi_instruction->repeat(data.size() / 2);
    _updi_instruction->st_ptr_inc16(data);
}

vector<uint8_t> UpdiApplication::read_data(uint32_t address,
                                           uint32_t byte_size) {
    if (byte_size > UPDI_MAX_REPEAT_SIZE) {
        throw UpdiException("Read size exceeds the limit in one go");
    }

    // Special case for only reading 1 byte
    if (byte_size == 1) {
        vector<uint8_t> data;
        data.push_back(_updi_instruction->ld(address));
        return data;
    }

    _updi_instruction->st_ptr(address);

    // Repeat to read the byte_size of bytes
    _updi_instruction->repeat(byte_size);
    return _updi_instruction->ld_ptr_inc(byte_size);
}

vector<uint8_t> UpdiApplication::read_data_words(uint32_t address,
                                                 uint32_t word_size) {
    if (word_size > UPDI_MAX_REPEAT_SIZE) {
        throw UpdiException("Read size exceeds the limit in one go");
    }

    // Special case for only reading 1 word
    if (word_size == 1) {
        return _updi_instruction->ld16(address);
    }

    _updi_instruction->st_ptr(address);

    // Repeat to read word_size of words
    _updi_instruction->repeat(word_size);
    return _updi_instruction->ld_ptr_inc16(word_size);
}

void UpdiApplication::write_fuse_data(uint32_t fuse_number, uint8_t value) {
    if (_pdi_v2) {
        throw UpdiException("PDI V2 is not supported now");
    }

    if (!in_prog_mode()) {
        throw UpdiException("Enter progmode first");
    }

    uint32_t        fuse_addr = fuse_number + _avr_device->get_fuses_addr();
    vector<uint8_t> data;
    data.push_back(fuse_addr & 0xff);
    write_data(_avr_device->get_nvmctrl_addr() + UPDI_NVMCTRL_ADDRL, data);

    data.clear();
    data.push_back((fuse_addr >> 8) & 0xff);
    write_data(_avr_device->get_nvmctrl_addr() + UPDI_NVMCTRL_ADDRH, data);

    data.clear();
    data.push_back(value);
    write_data(_avr_device->get_nvmctrl_addr() + UPDI_NVMCTRL_DATAL, data);

    execute_nvm_command(UPDI_V0_NVMCTRL_CTRLA_WRITE_FUSE);
}

uint8_t UpdiApplication::read_fuse_data(uint32_t fuse_number) {
    uint32_t fuse_addr = fuse_number + _avr_device->get_fuses_addr();
    auto     data = read_data(fuse_addr, 1);
    return data[0];
}

bool UpdiApplication::wait_unlocked(uint32_t timeout_ms) {
    auto start = system_clock::now();

    while (1) {
        uint8_t asi_status = _updi_instruction->ldcs(UPDI_ASI_SYS_STATUS);
        asi_status &= (1 << UPDI_ASI_SYS_STATUS_LOCKSTATUS);
        if (!asi_status) {
            return true;
        }

        auto duration =
            duration_cast<milliseconds>(system_clock::now() - start).count();
        if (duration > timeout_ms) {
            break;
        }
        usleep(5 * 1000);
    }

    cout << "Timeout waiting for device to unlock" << endl;
    return false;
}

void UpdiApplication::write_progmode_key() {
    if (in_prog_mode()) {
        cout << "Already in NVM programming mode" << endl;
        return;
    }

    _updi_instruction->key(UPDI_KEY_NVM);
    uint8_t key_status = _updi_instruction->ldcs(UPDI_ASI_KEY_STATUS);
    key_status &= (1 << UPDI_ASI_KEY_STATUS_NVMPROG);

    if (!key_status) {
        throw UpdiException("NVMPROG key is not accepted");
    }
}

bool UpdiApplication::wait_flash_ready() {
    cout << "Wait flash ready" << endl;

    // Timeout 10s
    auto start = system_clock::now();
    while (1) {
        uint8_t nvm_status = _updi_instruction->ld(
            _avr_device->get_nvmctrl_addr() + UPDI_NVMCTRL_STATUS);
        if ((nvm_status & (1 << UPDI_NVM_STATUS_WRITE_ERROR))) {
            cerr << "Flash has write error" << endl;
            return false;
        }

        if (!(nvm_status & ((1 << UPDI_NVM_STATUS_FLASH_BUSY) |
                            (1 << UPDI_NVM_STATUS_EEPROM_BUSY)))) {
            return true;
        }

        auto duration =
            duration_cast<milliseconds>(system_clock::now() - start).count();
        if (duration > 10 * 1000) {
            break;
        }
        usleep(100 * 1000);
    }

    return false;
}

void UpdiApplication::execute_nvm_command(uint8_t command) {
    cout << "Execute NVMCMD " << command << endl;

    _updi_instruction->st(_avr_device->get_nvmctrl_addr() + UPDI_NVMCTRL_CTRLA,
                          command);
}

}
