#include <glib-unix.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

#include "nvm_programmer.h"
#include "updi_common.h"

using namespace updi;
using namespace std;

static char*    device_name = nullptr;
static char*    com_port = nullptr;
static char*    hex_file = nullptr;
static gint     baud_rate = 0;
static gboolean chip_erase = false;
static gboolean chip_reset = false;
static gboolean read_chip_info = false;
static gint     write_fuse_number = -1;
static gint     read_fuse_number = -1;
static gint     fuse_value = -1;
static gboolean verbose = false;

static unique_ptr<NvmProgrammer> nvm = nullptr;

static GOptionEntry entries[] = {
    {"device", 'd', 0, G_OPTION_ARG_STRING, &device_name, "Target device",
     "tiny416"},
    {"comport", 'c', 0, G_OPTION_ARG_STRING, &com_port, "Com port to use",
     "/dev/ttyX"},
    {"baudrate", 'b', 0, G_OPTION_ARG_INT, &baud_rate, "Baud rate", "115200"},
    {"flash", 'f', 0, G_OPTION_ARG_STRING, &hex_file, "Intel HEX file to flash",
     nullptr},
    {"erase", 'e', 0, G_OPTION_ARG_NONE, &chip_erase,
     "Perform a chip erase (implied with --flash)", nullptr},
    {"reset", 'r', 0, G_OPTION_ARG_NONE, &chip_reset, "Reset chip", nullptr},
    {"info", 'i', 0, G_OPTION_ARG_NONE, &read_chip_info, "Read chip info",
     nullptr},

    // TODO: add verbose logging support
    {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output", nullptr},

    {"writefuse", 0, 0, G_OPTION_ARG_INT, &write_fuse_number, "Fuse to set",
     nullptr},
    {"fusebit", 0, 0, G_OPTION_ARG_INT, &fuse_value, "Fuse value to set",
     nullptr},
    {"readfuse", 0, 0, G_OPTION_ARG_INT, &read_fuse_number,
     "Read out the fuse-bits", nullptr},

    {nullptr}};

static int flash_file(const std::string& hexfile) {
    uint32_t     start_address = 0;
    auto         device = nvm->get_device();
    IntelHexFile ihex(device->get_flash_size(), device->get_flash_pagesize());

    try {
        start_address = ihex.load_file(hexfile);
    } catch (const ios_base::failure& e) {
        cerr << "Failed to load hex file. Exception: " << e.what() << endl;
        return -1;
    }

    nvm->chip_erase();

    auto pages = ihex.get_page_data();
    nvm->write_flash(start_address, pages);

    // Read out pages of flash again
    // This is to verify if flashing is successful
    vector<uint8_t> flash_data = nvm->read_flash(
        device->get_flash_start_addr(), pages.size() * device->get_flash_pagesize());
    auto hex_data = ihex.get_flash_data();

    if (!std::equal(hex_data.begin(), hex_data.end(), flash_data.begin())) {
        cerr << "Flash verification error" << endl;
        return -1;
    }

    cout << "Programming successful" << endl;
    return 0;
}

int main(int argc, char** argv) {
    GError*         error = nullptr;
    GOptionContext* optctx;
    int result = 0;

    optctx = g_option_context_new("Command line tool for UPDI programming");
    g_option_context_add_main_entries(optctx, entries, nullptr);
    if (!g_option_context_parse(optctx, &argc, &argv, &error)) {
        g_printerr("Error parsing options: %s\n", error->message);
        g_option_context_free(optctx);
        g_clear_error(&error);
        return -1;
    }

    if (!(device_name && com_port) || !baud_rate ||
        !(hex_file != nullptr || chip_erase || chip_reset || read_chip_info ||
          write_fuse_number || read_fuse_number)) {
        cerr << "No valid action (erase, flash, reset, read/write fuses or info)" << endl;
        return -1;
    }

    string supported_devices = AvrDevice::get_supported_devices();
    if (supported_devices.find(device_name) == string::npos) {
        cerr << "Device " << device_name << " is not supported" << endl;
        cerr << "Current supported list: " << supported_devices << endl;
        return -1;
    }    

    if ((write_fuse_number >=0 || read_fuse_number >= 0) && fuse_value < 0) {
        cerr << "Invalid fuse value" << endl;
        return -1;
    }

    nvm = make_unique<NvmProgrammer>(com_port, baud_rate, device_name);

    if (!chip_reset) {
        string sib_str = nvm->get_device_info();
        cout << "SIB: " << sib_str << endl;

        try {
            nvm->enter_progmode();
        } catch (const UpdiException& e) {
            cerr << "Device is locked. Perform unlock with chip erase first"
                 << endl;
            nvm->unlock_device();
        }

        if (chip_erase) {
            try {
                nvm->chip_erase();
            } catch (const UpdiException& e) {
                cerr << "Failed to erase chip. Exception: " << e.what() << endl;
                return -1;
            }
        }

        if (hex_file) {
            result = flash_file(hex_file);
        } else {
            if (write_fuse_number >= 0) {
                try {
                    nvm->write_fuse(write_fuse_number, fuse_value);
                } catch (const UpdiException& e) {
                    cerr << "Failed to write fuse " << write_fuse_number << " "
                         << e.what() << endl;
                    return -1;
                }
            } else if (read_fuse_number >= 0) {
                try {
                    uint8_t fuse_value = nvm->read_fuse(read_fuse_number);
                    cout << "Fuse " << read_fuse_number << " value is " << hex
                         << fuse_value << dec << endl;
                } catch (const UpdiException& e) {
                    cerr << "Failed to read fuse " << read_fuse_number << " "
                         << e.what() << endl;
                    return -1;
                }
            } else {
                cout << "Ready to quit UPDI programmer" << endl;;
            }
        }
    }

    nvm->leave_progmode();
    return result;
}