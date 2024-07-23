#ifndef UART_HPP
#define UART_HPP

#include <functional>
#include <deque>
#include <mutex>
#include <stdint.h>
#include "config.hpp"

class UART_RX
{
public:
    UART_RX(std::function<void(uint8_t)> get_byte) : get_byte(get_byte), state(WAIT_FOR_START), bit_count(0), sample_count(0), received_byte(0) {}
    void put_samples(const unsigned int* buffer, unsigned int n);

private:
    std::function<void(uint8_t)> get_byte;

    // Estados da máquina de estados
    enum State {
        WAIT_FOR_START,
        RECEIVE_BITS,
        CHECK_STOP
    };

    State state;
    int bit_count;
    int sample_count;
    uint8_t received_byte;
    std::deque<unsigned int> sample_buffer;

    // Constantes
    static const int SAMPLES_PER_BIT = 160;
    static const int START_BIT_THRESHOLD = 25;

    bool check_start_bit();
};

class UART_TX
{
public:
    void put_byte(uint8_t byte);
    void get_samples(unsigned int* buffer, unsigned int n);

private:
    std::deque<unsigned int> samples;
    std::mutex samples_mutex;
    void put_bit(unsigned int bit);
};

#endif
