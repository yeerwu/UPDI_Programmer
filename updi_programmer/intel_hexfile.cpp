#include "intel_hexfile.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <string.h>

using namespace std;

namespace updi {

static uint64_t asciiHexTo64(string s) {
    stringstream o;
    o << hex << s;
    uint64_t v;
    o >> v;
    return v;
}

IntelHexFile::IntelHexFile(uint32_t flash_size, uint32_t page_size)
    : nvm_flash_size(flash_size), nvm_page_size(page_size) {
    nvm_data.resize(nvm_flash_size);
}

IntelHexFile::~IntelHexFile() {
}

int IntelHexFile::load_file(const string& filename) {
    char     rec_line[524] = {0};
    string   parse_error;
    ifstream file(filename.c_str());
    int      start_address = 0x7fffffff;

    if (!file.is_open()) {
        stringstream ss;
        ss << "failed to open file " << filename;
        throw ios_base::failure(ss.str());
    }

    firmware_size = 0;

    while (true) {
        file.getline(rec_line, 524);
        string record = rec_line;

        /*
         * End of file
         */
        if (!file || record.empty()) {
            break;
        }

        // For AVR tiny series, we only support record type 0 and 1.
        // x86 related record types will not be handled here
        int rec_start = parse_record(record, parse_error);
        if (rec_start < 0) {
            cerr << parse_error << endl;
            throw ios_base::failure(parse_error);
        }

        // Normally, first record should contain the start address.
        // This is just in case records are not organized in order
        if (rec_start < start_address) {
            start_address = rec_start;
        }
        memset(rec_line, 0, 524);
    }

    cout << "total size " << firmware_size << endl;
    // Align binary data on page size
    if ((firmware_size % nvm_page_size) != 0) {
        uint32_t rounded_size =
            ((firmware_size + nvm_page_size - 1) / nvm_page_size) *
            nvm_page_size;
        nvm_data.resize(rounded_size);
    }

    cout << "after alignment, total size " << nvm_data.size() << endl;

    // Split data into progam pages
    for (size_t i = 0; i < nvm_data.size(); i += nvm_page_size) {
        vector<uint8_t> pageData(nvm_data.begin() + i,
                                 nvm_data.begin() + i + nvm_page_size);
        ProgramPage     p;
        p.address = i;
        p.pageSize = nvm_page_size;
        p.data.swap(pageData);
        nvm_pages.push_back(p);
    }

    return start_address;
}

const vector<ProgramPage>& IntelHexFile::get_page_data() {
    return nvm_pages;
}

const std::vector<uint8_t>& IntelHexFile::get_flash_data() {
    return nvm_data;
}

int IntelHexFile::parse_record(const std::string& record, string& error) {
    if (record[0] != ':') {
        error = "Wrong start of record";
        return -1;
    }

    uint8_t  count = 2 * asciiHexTo64(record.substr(1, 2));
    uint8_t  high_addr = asciiHexTo64(record.substr(3, 2));
    uint8_t  low_addr = asciiHexTo64(record.substr(5, 2));
    uint16_t start_addr = ((high_addr << 8) | low_addr);
    uint8_t  recordType = asciiHexTo64(record.substr(7, 2));

    if (recordType != 0 && recordType != 1) {
        error = "Unsupported record type";
        return -1;  // Unsupported record type
    }

    uint8_t cChecksum = (count / 2) + high_addr + low_addr + recordType;

    if (count != record.length() - (9 + 2 + 1)) {
        error = "Wrong data size";
        return -1;  // record data size is wrong
    }

    // Fill the nvm_data
    for (uint8_t i = 0; i < count; i += 2) {
        uint8_t v = asciiHexTo64(record.substr(9 + i, 2));
        cChecksum += v;
        if ((start_addr + i / 2) > nvm_flash_size) {
            error = "Exceed maximum flash size";
            return -1;  // exceed maximum flash size
        }

        nvm_data[start_addr + i / 2] = v;
        firmware_size++;
    }

    uint8_t checksum = asciiHexTo64(record.substr(record.length() - 3, 2));

    // Verify if checksum is 2's complement
    if (((uint8_t)(cChecksum + checksum)) != 0) {
        error = "Failed to verify checksum";
        return -1;
    }

    return start_addr;
}

}  // namespace updi