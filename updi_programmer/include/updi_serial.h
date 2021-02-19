#ifndef __UPDI_SERIAL_H__
#define __UPDI_SERIAL_H__

#include <stdint.h>

#include <iostream>
#include <string>
#include <vector>

namespace updi {

/*
 * @brief The UPDI serial communication class
 *
 * This class is implementing low level physical UART communication.
 * High level applications can adopt this class to send/receive data.
 * Interfaces will be invoked by @ref UpdiInstruction
 */
class UpdiSerial {
   public:
    UpdiSerial(const std::string& port, uint32_t baud_rate);
    ~UpdiSerial();

    /*
     * @brief send an array of bytes to the MCU
     *
     * @param[in] command byte array to send out
     */
    void send(const std::vector<uint8_t>& command);

    /*
     * @brief receive an array of bytes from the MCU
     *
     * @param[out] data byte array to store received data
     * @param[in] expected_size total number of bytes to receive
     */
    void receive(std::vector<uint8_t>& data, uint32_t expected_size);

    /*
     * @brief send a double break to reset the UDPI port
     *
     * This should be called when UDPI is not working as expected.
     * It will re-init UDPI state machine so MCU will go to an initial state.
     */
    void send_double_break();

   private:
    bool init_serial_comm(uint32_t baud);

    std::string _serial_port;
    uint32_t    _baud_rate;
    int         _serial_fd;
};

}  // namespace updi

#endif