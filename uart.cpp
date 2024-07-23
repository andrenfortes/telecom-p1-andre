#include "uart.hpp"

void UART_RX::put_samples(const unsigned int* buffer, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++) {
        sample_buffer.push_back(buffer[i]);

        switch (state) {
        case WAIT_FOR_START:
            if (sample_buffer.size() >= 30) {
                if (check_start_bit()) {
                    state = RECEIVE_BITS;
                    bit_count = 0;
                    sample_count = 0;
                    received_byte = 0;
                }
                sample_buffer.pop_front();
            }
            break;

        case RECEIVE_BITS:
            if (sample_count == SAMPLES_PER_BIT / 2) {
                if (bit_count < 8) {
                    received_byte >>= 1;
                    if (sample_buffer.front() == 1) {
                        received_byte |= 0x80;
                    }
                    bit_count++;
                }
                else {
                    state = CHECK_STOP;
                }
            }
            sample_count++;
            if (sample_count >= SAMPLES_PER_BIT) {
                sample_count = 0;
                sample_buffer.pop_front();
            }
            break;

        case CHECK_STOP:
            if (sample_count == SAMPLES_PER_BIT / 2) {
                if (sample_buffer.front() == 1) {
                    get_byte(received_byte);
                }
                state = WAIT_FOR_START;
            }
            sample_count++;
            if (sample_count >= SAMPLES_PER_BIT) {
                sample_count = 0;
                sample_buffer.pop_front();
            }
            break;
        }
    }
}

bool UART_RX::check_start_bit()
{
    int low_samples = 0;
    for (int i = 0; i < 30; i++) {
        if (sample_buffer[i] == 0) {
            low_samples++;
        }
    }
    return low_samples >= START_BIT_THRESHOLD;
}

void UART_TX::put_byte(uint8_t byte)
{
    samples_mutex.lock();
    put_bit(0);  // start bit
    for (int i = 0; i < 8; i++) {
        put_bit(byte & 1);
        byte >>= 1;
    }
    put_bit(1);  // stop bit
    samples_mutex.unlock();
}

void UART_TX::get_samples(unsigned int* buffer, unsigned int n)
{
    samples_mutex.lock();
    std::vector<unsigned int>::size_type i = 0;
    while (!samples.empty() && i < n) {
        buffer[i++] = samples.front();
        samples.pop_front();
    }
    samples_mutex.unlock();

    while (i < n) {
        // idle
        buffer[i++] = 1;
    }
}

void UART_TX::put_bit(unsigned int bit)
{
    for (int i = 0; i < SAMPLES_PER_SYMBOL; i++) {
        samples.push_back(bit);
    }
}
