#include "updi_instruction_set.h"

#include <iostream>
#include <memory>

#include "updi_common.h"

using namespace std;

namespace updi {

UpdiInstruction::UpdiInstruction(const string& port, uint32_t baud_rate)
    : _use_24bit_addr(false) {
    _serial_comm = std::make_unique<UpdiSerial>(port, baud_rate);
    _serial_comm->send_double_break();
    init();

    if (!updi_is_ready()) {
        _serial_comm->send_double_break();

        // Re-init UDPI
        init();
    }
}

UpdiInstruction::~UpdiInstruction() {
}

uint8_t UpdiInstruction::ldcs(uint8_t reg_addr) {
    vector<uint8_t> request;
    vector<uint8_t> response;

    request.push_back(UPDI_PHY_SYNC);
    request.push_back((UPDI_LDCS | (reg_addr & 0x0F)));
    _serial_comm->send(request);

    _serial_comm->receive(response, 1);
    if (response.size() != 1) {
        throw UpdiException("Error with ldcs");
    }

    return response[0];
}

void UpdiInstruction::stcs(uint8_t reg_address, uint8_t value) {
    vector<uint8_t> request;
    request.push_back(UPDI_PHY_SYNC);
    request.push_back((UPDI_STCS | (reg_address & 0x0F)));
    request.push_back(value);

    _serial_comm->send(request);
}

uint8_t UpdiInstruction::ld(uint32_t address) {
    vector<uint8_t> data;
    vector<uint8_t> response;
    data.push_back(UPDI_PHY_SYNC);

    if (_use_24bit_addr) {
        data.push_back(UPDI_LDS | UPDI_ADDRESS_24 | UPDI_DATA_8);
        data.push_back(address & 0xFF);
        data.push_back((address >> 8) & 0xFF);
        data.push_back((address >> 16) & 0xFF);
    } else {
        data.push_back(UPDI_LDS | UPDI_ADDRESS_16 | UPDI_DATA_8);
        data.push_back(address & 0xFF);
        data.push_back((address >> 8) & 0xFF);
    }

    _serial_comm->send(data);
    _serial_comm->receive(response, 1);

    if (response.size() != 1) {
        throw UpdiException("Error with ld");
    }

    return response[0];
}

std::vector<uint8_t> UpdiInstruction::ld16(uint32_t address) {
    vector<uint8_t> data;
    vector<uint8_t> response;
    data.push_back(UPDI_PHY_SYNC);

    if (_use_24bit_addr) {
        data.push_back(UPDI_LDS | UPDI_ADDRESS_24 | UPDI_DATA_16);
        data.push_back(address & 0xFF);
        data.push_back((address >> 8) & 0xFF);
        data.push_back((address >> 16) & 0xFF);
    } else {
        data.push_back(UPDI_LDS | UPDI_ADDRESS_16 | UPDI_DATA_16);
        data.push_back(address & 0xFF);
        data.push_back((address >> 8) & 0xFF);
    }

    _serial_comm->send(data);
    _serial_comm->receive(response, 2);

    if (response.size() != 2) {
        throw UpdiException("Error with ld16");
    }

    return response;
}

void UpdiInstruction::st(uint32_t address, uint8_t value) {
    vector<uint8_t> data;
    vector<uint8_t> response;
    data.push_back(UPDI_PHY_SYNC);

    if (_use_24bit_addr) {
        data.push_back(UPDI_STS | UPDI_ADDRESS_24 | UPDI_DATA_8);
        data.push_back(address & 0xFF);
        data.push_back((address >> 8) & 0xFF);
        data.push_back((address >> 16) & 0xFF);
    } else {
        data.push_back(UPDI_STS | UPDI_ADDRESS_16 | UPDI_DATA_8);
        data.push_back(address & 0xFF);
        data.push_back((address >> 8) & 0xFF);
    }

    _serial_comm->send(data);

    // Wait for ACK
    _serial_comm->receive(response, 1);
    if (response.size() != 1 || response[0] != UPDI_PHY_ACK) {
        cerr << "Error with st instruction (address)" << endl;
        throw UpdiException("Error with st");
    }

    data.clear();
    response.clear();

    data.push_back(value);
    _serial_comm->send(data);

    // Wait for ACK
    _serial_comm->receive(response, 1);
    if (response.size() != 1 || response[0] != UPDI_PHY_ACK) {
        cerr << "Error with st instruction (data)" << endl;
        throw UpdiException("Error with st");
    }
}

void UpdiInstruction::st16(uint32_t address, uint16_t value) {
    vector<uint8_t> data;
    vector<uint8_t> response;
    data.push_back(UPDI_PHY_SYNC);

    if (_use_24bit_addr) {
        data.push_back(UPDI_STS | UPDI_ADDRESS_24 | UPDI_DATA_16);
        data.push_back(address & 0xFF);
        data.push_back((address >> 8) & 0xFF);
        data.push_back((address >> 16) & 0xFF);
    } else {
        data.push_back(UPDI_STS | UPDI_ADDRESS_16 | UPDI_DATA_16);
        data.push_back(address & 0xFF);
        data.push_back((address >> 8) & 0xFF);
    }

    _serial_comm->send(data);

    // Wait for ACK
    _serial_comm->receive(response, 1);
    if (response.size() != 1 || response[0] != UPDI_PHY_ACK) {
        cerr << "Error with st instruction (address)" << endl;
        throw UpdiException("Error with st16");
    }

    data.clear();
    response.clear();

    data.push_back(value & 0xFF);
    data.push_back((value >> 8) & 0xFF);
    _serial_comm->send(data);

    // Wait for ACK
    _serial_comm->receive(response, 1);
    if (response.size() != 1 || response[0] != UPDI_PHY_ACK) {
        cerr << "Error with st instruction (data)" << endl;
        throw UpdiException("Error with st16");
    }
}

std::vector<uint8_t> UpdiInstruction::ld_ptr_inc(uint32_t size) {
    vector<uint8_t> data;
    vector<uint8_t> response;
    data.push_back(UPDI_PHY_SYNC);
    data.push_back(UPDI_LD | UPDI_PTR_INC | UPDI_DATA_8);
    _serial_comm->send(data);

    _serial_comm->receive(response, size);
    if (response.size() != size) {
        cerr << "Error with ld_ptr_inc. Actual recevied " << response.size()
             << endl;
        throw UpdiException("Error with ld_ptr_inc");
    }

    return response;
}

std::vector<uint8_t> UpdiInstruction::ld_ptr_inc16(uint32_t size) {
    vector<uint8_t> data;
    vector<uint8_t> response;
    data.push_back(UPDI_PHY_SYNC);
    data.push_back(UPDI_LD | UPDI_PTR_INC | UPDI_DATA_16);
    _serial_comm->send(data);

    _serial_comm->receive(response, size * 2);
    if (response.size() != size * 2) {
        cerr << "Error with ld_ptr_inc16. Actual recevied " << response.size()
             << endl;
        throw UpdiException("Error with ld_ptr_inc16");
    }

    return response;
}

void UpdiInstruction::st_ptr(uint32_t address) {
    vector<uint8_t> data;
    vector<uint8_t> response;
    data.push_back(UPDI_PHY_SYNC);

    if (_use_24bit_addr) {
        data.push_back(UPDI_ST | UPDI_PTR_ADDRESS | UPDI_DATA_24);
        data.push_back(address & 0xFF);
        data.push_back((address >> 8) & 0xFF);
        data.push_back((address >> 16) & 0xFF);
    } else {
        data.push_back(UPDI_ST | UPDI_PTR_ADDRESS | UPDI_DATA_16);
        data.push_back(address & 0xFF);
        data.push_back((address >> 8) & 0xFF);
    }

    _serial_comm->send(data);

    // Wait for ACK
    _serial_comm->receive(response, 1);
    if (response.size() != 1 || response[0] != UPDI_PHY_ACK) {
        cerr << "Error with st_ptr instruction" << endl;
        throw UpdiException("Error with st_prt");
    }
}

void UpdiInstruction::st_ptr_inc(const std::vector<uint8_t>& data) {
    vector<uint8_t> cmd;
    vector<uint8_t> response;
    cmd.push_back(UPDI_PHY_SYNC);

    cmd.push_back(UPDI_ST | UPDI_PTR_INC | UPDI_DATA_8);
    cmd.push_back(data[0]);

    // Send first byte
    _serial_comm->send(data);

    // Wait for ACK
    _serial_comm->receive(response, 1);
    if (response.size() != 1 || response[0] != UPDI_PHY_ACK) {
        cerr << "Error with st ptr_inc instruction" << endl;
        throw UpdiException("Ack error with st_ptr_inc");
    }

    // Send other bytes
    size_t n = 1;
    while (n < data.size()) {
        vector<uint8_t> tmp;
        vector<uint8_t> tmp_ack;
        tmp.push_back(data[n]);
        _serial_comm->send(tmp);

        // Wait for ACK
        _serial_comm->receive(tmp_ack, 1);
        if (tmp_ack.size() != 1 || tmp_ack[0] != UPDI_PHY_ACK) {
            throw UpdiException("Error with st_ptr_inc");
        }

        tmp.clear();
        tmp_ack.clear();
        n++;
    }
}

void UpdiInstruction::st_ptr_inc16(const std::vector<uint8_t>& data) {
    vector<uint8_t> cmd;
    vector<uint8_t> response;
    uint8_t         ctrla_ackon = 1 << UPDI_CTRLA_IBDLY_BIT;
    uint8_t         ctrla_ackoff = ctrla_ackon | (1 << UPDI_CTRLA_RSD_BIT);

    // Disable response signature
    // This is to reduce latency
    stcs(UPDI_CS_CTRLA, ctrla_ackoff);

    cmd.push_back(UPDI_PHY_SYNC);
    cmd.push_back(UPDI_ST | UPDI_PTR_INC | UPDI_DATA_16);
    _serial_comm->send(cmd);

    // No response expected
    _serial_comm->send(data);

    // Re-enable acks
    stcs(UPDI_CS_CTRLA, ctrla_ackon);
}

void UpdiInstruction::repeat(uint32_t repeats) {
    repeats -= 1;

    vector<uint8_t> data;
    data.push_back(UPDI_PHY_SYNC);
    data.push_back(UPDI_REPEAT | UPDI_REPEAT_BYTE);
    data.push_back((repeats & 0xFF));

    _serial_comm->send(data);
}

string UpdiInstruction::read_sib() {
    vector<uint8_t> data;
    vector<uint8_t> response;
    string          sib;

    data.push_back(UPDI_PHY_SYNC);
    data.push_back(UPDI_KEY | UPDI_KEY_SIB | UPDI_SIB_16BYTES);
    _serial_comm->send(data);

    _serial_comm->receive(response, 16);
    sib.append(response.begin(), response.end());
    sib.push_back('\0');
    return sib;
}

void UpdiInstruction::key(const std::string& key) {
    vector<uint8_t> data;
    data.push_back(UPDI_PHY_SYNC);
    data.push_back(UPDI_KEY | UPDI_KEY_KEY | UPDI_KEY_64);

    _serial_comm->send(data);

    // Send reversed key characters
    data.clear();
    data.insert(data.begin(), key.rbegin(), key.rend());
    _serial_comm->send(data);
}

void UpdiInstruction::init() {
    // Disable collision detection and enable inter-byte delay
    stcs(UPDI_CS_CTRLB, 1 << UPDI_CTRLB_CCDETDIS_BIT);
    stcs(UPDI_CS_CTRLA, 1 << UPDI_CTRLA_IBDLY_BIT);
}

bool UpdiInstruction::updi_is_ready() {
    if (ldcs(UPDI_CS_STATUSA) != 0) {
        return true;
    }

    cerr << "UPDI is not ready - reinitialisation required" << endl;
    return false;
}

}  // namespace updi
