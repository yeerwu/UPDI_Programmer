#include "nvm_programmer.h"

#include <iostream>

#include "updi_common.h"

using namespace std;

namespace updi {

NvmProgrammer::NvmProgrammer(const std::string& port,
                             uint32_t           baud_rate,
                             const std::string& device_name)
    : _programming(false) {
    _avr_device = make_shared<AvrDevice>(device_name);
    _updi_application =
        make_unique<UpdiApplication>(port, baud_rate, _avr_device);
}

NvmProgrammer::~NvmProgrammer() {
}

std::string NvmProgrammer::get_device_info() {
    return _updi_application->init_nvm_operation();
}

void NvmProgrammer::enter_progmode() {
    cout << "Enter NVM programming mode" << endl;
    _updi_application->enter_progmode();
    _programming = true;
}

void NvmProgrammer::leave_progmode() {
    cout << "Leave NVM programming mode" << endl;    
    _updi_application->leave_progmode();
    _programming = false;
}

void NvmProgrammer::unlock_device() {
    if (_programming) {
        cout << "Device already unlocked" << endl;
        return;
    }

    _updi_application->unlock();
    _programming = true;
}

void NvmProgrammer::chip_erase() {
    if (!_programming) {
        throw UpdiException("Enter progmode first");
    }

    _updi_application->chip_erase();
}

void NvmProgrammer::write_flash(uint32_t                        address,
                                const std::vector<ProgramPage>& pages) {
    uint32_t page_start_addr = address;

    if (!_programming) {
        throw UpdiException("Enter progmode first");
    }
    
    // Map the page starting address to the real flash address space
    // e.g., for tiny416, mapped flash start address = 0x8000
    if (page_start_addr < _avr_device->get_flash_start_addr()) {
        page_start_addr += _avr_device->get_flash_start_addr();
    }

    for (auto page : pages) {
        cout << "Write page at " << hex << page_start_addr << dec << endl;
        _updi_application->write_nvm_page(page_start_addr, page.data);
        page_start_addr += page.pageSize;
    }
}

vector<uint8_t> NvmProgrammer::read_flash(uint32_t address, uint32_t size) {
    vector<uint8_t> flash_data;
    uint32_t        page_start_addr = address;

    if (!_programming) {
        throw UpdiException("Enter progmode first");
    }

    if ((size % _avr_device->get_flash_pagesize())) {
        throw UpdiException("Only full page aligned flash supported");
    }

    uint32_t page_count = size / _avr_device->get_flash_pagesize();
    for (uint32_t i = 0; i < page_count; i++) {
        auto page_data = _updi_application->read_data_words(
            page_start_addr, _avr_device->get_flash_pagesize() / 2);
        flash_data.insert(flash_data.end(), page_data.begin(), page_data.end());
        page_start_addr += _avr_device->get_flash_pagesize();
    }

    return flash_data;
}

uint8_t NvmProgrammer::read_fuse(uint32_t fuse_num) {
    if (!_programming) {
        throw UpdiException("Enter progmode first");
    }

    return _updi_application->read_fuse_data(fuse_num);
}

void NvmProgrammer::write_fuse(uint32_t fuse_num, uint8_t value) {
    if (!_programming) {
        throw UpdiException("Enter progmode first");
    }

    _updi_application->write_fuse_data(fuse_num, value);
}

}  // namespace updi