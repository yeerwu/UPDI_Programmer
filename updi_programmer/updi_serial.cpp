#include "updi_serial.h"

#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <iostream>

#include "updi_common.h"

using namespace std;

namespace updi {

UpdiSerial::UpdiSerial(const std::string& port, uint32_t baud_rate)
    : _serial_port(port), _baud_rate(baud_rate) {
    // Open serial comm and setup baud rate/parity/stop bits
    if (init_serial_comm(_baud_rate)) {
        // Send a break as handshake
        vector<uint8_t> command;
        command.push_back(UPDI_BREAK);
        send(command);
    }
}

UpdiSerial::~UpdiSerial() {
    // Close serial comm
    close(_serial_fd);
}

void UpdiSerial::send(const vector<uint8_t>& command) {
    vector<uint8_t> echo;
    echo.resize(command.size());
    write(_serial_fd, &command[0], command.size());

    // read the echo
    read(_serial_fd, &echo[0], echo.size());
}

void UpdiSerial::receive(vector<uint8_t>& data, uint32_t expected_size) {
    uint32_t read_count = 0;
    data.resize(expected_size);

    while (read_count < expected_size) {
        int num_bytes =
            read(_serial_fd, &data[read_count], expected_size - read_count);
        if (num_bytes < 0) {
            cerr << "Error reading: " << strerror(errno) << endl;
            data.clear();
            break;
        }

        read_count += num_bytes;
    }
}

void UpdiSerial::send_double_break() {
    // Re-init at a lower baud
    // At 300 bauds, the break character will pull the line low for 30ms
    close(_serial_fd);
    if (init_serial_comm(300)) {
        vector<uint8_t> double_break;
        double_break.push_back(UPDI_BREAK);
        double_break.push_back(UPDI_BREAK);

        send(double_break);
    }

    // Re-init at the real baud
    close(_serial_fd);
    init_serial_comm(_baud_rate);
}

bool UpdiSerial::init_serial_comm(uint32_t baud) {
    struct termios tty;
    _serial_fd = open(_serial_port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

    if (tcgetattr(_serial_fd, &tty) != 0) {
        cerr << "Error from tcgetattr " << strerror(errno) << endl;
        return false;
    }

    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag |= PARENB;    // enable parity
    tty.c_cflag &= ~PARODD;   // use Even parity
    tty.c_cflag &= ~CRTSCTS;  // turn off rts/cts hardware flow ctrl
    tty.c_cflag |= CSTOPB;    // two Stop bits

    tty.c_lflag &= ~(ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK | ECHONL);

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // turn off s/w flow ctrl
    // Disable any special handling of received bytes
    tty.c_iflag &=
        ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    tty.c_oflag &= ~(OPOST | ONLCR | OCRNL);

    // 1s timeout
    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = 0;

    switch (baud) {
        case 300:
            cfsetispeed(&tty, B300);
            cfsetospeed(&tty, B300);
            break;
        case 9600:
            cfsetispeed(&tty, B9600);
            cfsetospeed(&tty, B9600);
            break;
        case 19200:
            cfsetispeed(&tty, B19200);
            cfsetospeed(&tty, B19200);
            break;
        case 38400:
            cfsetispeed(&tty, B38400);
            cfsetospeed(&tty, B38400);
            break;
        default:
            cfsetispeed(&tty, B115200);
            cfsetospeed(&tty, B115200);
            break;
    }

    if (tcsetattr(_serial_fd, TCSANOW, &tty) != 0) {
        cerr << "Error from tcssetattr " << strerror(errno) << endl;
        return false;
    }

    return true;
}

}  // namespace updi