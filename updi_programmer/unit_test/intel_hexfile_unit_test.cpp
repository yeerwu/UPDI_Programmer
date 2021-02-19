#include <glib.h>
#include <gmodule.h>
#include <stdlib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "intel_hexfile.h"

using namespace std;
namespace updi {

using testing::AllOf;
using testing::Gt;
using testing::Le;
using testing::Lt;
using testing::Values;

#define FLASH_SIZE 4 * 1024
#define TINY_FLASH_SIZE 1024
#define FLASH_PAGE_SIZE 64

TEST(IntelHexFileTest, ParseFileSuccessfully) {
    IntelHexFile hex_file(FLASH_SIZE, FLASH_PAGE_SIZE);
    int          start_address = hex_file.load_file("/usr/bin/pasa_attiny.hex");

    // flash start offset is 0
    // As the firmware is 2660 bytes
    // total page number is ROUND(2660, 64)/64 = 42
    EXPECT_EQ(0, start_address) << "Start offset is " << start_address;
    EXPECT_EQ((size_t)42, hex_file.get_page_data().size())
        << "Actual page number is " << hex_file.get_page_data().size();
}

// Load a pre-defined wrong intel hex file
// and fail to parse the file.
// Expect class to throw an exception
TEST(IntelHexFileTest, WrongHexFile) {
    IntelHexFile hex_file(FLASH_SIZE, FLASH_PAGE_SIZE);

    EXPECT_THROW(hex_file.load_file("/usr/bin/wrong_checksum.hex"), ios_base::failure)
        << "No exception is thrown";
}

// Load a pre-defined correct intel hex file
// but initial flash size to a small value.
// Expect class to throw an exception
TEST(IntelHexFileTest, FlashSizeTooSmall) {
    IntelHexFile hex_file(TINY_FLASH_SIZE, FLASH_PAGE_SIZE);

    EXPECT_THROW(hex_file.load_file("/usr/bin/pasa_attiny.hex"), ios_base::failure)
        << "No exception is thrown";
}

}  // namespace updi