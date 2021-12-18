#include <bitset>
#include <stdio.h>
#include <string.h>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

static const uint16_t INPUT_MAX = 2048;
static const uint16_t MAX_BITS = 1366 * 4;
static const uint8_t SUB_RESULTS_MAX = 64;

typedef struct Transmission {
    void init();
    void destroy();
    bool add_bits(const char* str);
    uint64_t parse();
    uint16_t version_sum() const { return _version_sum; }
    
private:
    bool read_n(const uint8_t n, uint8_t* value);
    bool read_nn(const uint8_t nn, uint16_t* value);
    uint8_t parse_litteral(uint64_t* value);
    uint64_t parse_packet(uint64_t* result);

    // std::bitset is actually pretty fast
    std::bitset<MAX_BITS> _bits;
    uint16_t _it;
    uint16_t _version_sum;
    uint16_t _packet_size;

} Transmission;

void Transmission::init() {
    _it = 0;
    _version_sum = 0;
    _packet_size = 0;
}

void Transmission::destroy() {
    // nothing
}

bool Transmission::read_n(const uint8_t n, uint8_t* value) {
    if (_it + n - 1 >= _packet_size) return false;

    *value = 0;
    for (uint8_t i = n; i > 0; --i) {
        *value <<= 1;
        *value += _bits[_it];
        _it += 1;
    }
    return true;
}

bool Transmission::read_nn(const uint8_t nn, uint16_t* value) {
    if (_it + nn- 1 >= _packet_size) return false;

    *value = 0;
    for (uint8_t i = nn; i > 0; --i) {
       *value <<= 1;
       *value += _bits[_it];
        _it += 1;
    }
    return true;
}

uint8_t Transmission::parse_litteral(uint64_t* value) {
    uint8_t more = 1;
    uint8_t n_read = 0;
    *value = 0;

    while (more) {
        read_n(1, &more);
        *value <<= 4;
        uint8_t v = 0;
        read_n(4, &v);
        *value += v;
        n_read += 5;
    }
    return n_read;
}

uint64_t Transmission::parse_packet(uint64_t* result) {
    uint8_t version;
    uint16_t read = 0;
    uint64_t packet_value = 0;
    if (!read_n(3, &version)) return read;
    read += 3;
    _version_sum += version;

    uint8_t packet_type;
    if (!read_n(3, &packet_type)) return read;
    read += 3;

    // bail out as soon as possible if we have a type 4
    if (packet_type == 4) {
        uint8_t n_read = parse_litteral(&packet_value);
        read += n_read;
        *result = packet_value;
        return read;
    }

    // otherwise, we got an operator packet
    // let's retrieve all the sub packets first

    uint64_t sub_results[SUB_RESULTS_MAX];
    uint8_t n_results = 0;

    uint8_t length_type_id;
    if(!read_n(1, &length_type_id)) return read;
    read += 1;

    if (length_type_id == 0) {
        // length type 0
        // 15 bits number indicates how many bits are in the sub packets
        uint16_t length;
        if(!read_nn(15, &length)) return read;
        read += 15;

        uint16_t total = 0;
        while (total < length) {
            total += parse_packet(&sub_results[n_results]);
            if (n_results++ == SUB_RESULTS_MAX) return read;
        }
        read += total;
    } else { 
        // length type 1
        // 11 bits number indicates how many sub packets there are
        uint16_t length;
        if(!read_nn(11, &length)) return read;
        read += 11;
        while (length > 0) {
            read += parse_packet(&sub_results[n_results]);
            if (n_results++ == SUB_RESULTS_MAX) return read;
            length -= 1;
        }
    }

    // Now calculate the packet value according to the operator
    switch (packet_type) {
        case 0:
            for (uint8_t i = 0; i < n_results; ++i)
                packet_value += sub_results[i];
            break;
        case 1:
            packet_value = 1;
            for (uint8_t i = 0; i < n_results; ++i)
                packet_value *= sub_results[i];
            break;
        case 2:
            packet_value = UINT64_MAX;
            for (uint8_t i = 0; i < n_results; ++i)
                if (sub_results[i] < packet_value) packet_value = sub_results[i];
            break;
        case 3:
            for (uint8_t i = 0; i < n_results; ++i)
                if (sub_results[i] > packet_value) packet_value = sub_results[i];
            break;
        case 5:
            if (n_results < 2) break;
            packet_value = (sub_results[0] > sub_results[1])? 1 : 0;
            break;
        case 6:
            if (n_results < 2) break;
            packet_value = (sub_results[0] < sub_results[1])? 1 : 0;
            break;
        case 7:
            if (n_results < 2) break;
            packet_value = (sub_results[0] == sub_results[1])? 1 : 0;
            break;
        default: return 0;
    }

    *result = packet_value;
    return read;
}

uint64_t Transmission::parse() {
    uint64_t result = 0;
    parse_packet(&result);
    return result;
}

bool Transmission::add_bits(const char* str) {
    uint16_t it = 0;
    while (str[it] != 0) {
        const uint16_t pos = it*4;
        if (pos > MAX_BITS) return false;
        switch (str[it]) {
            case '0': break;
            case '1':
                _bits.set(pos + 3, 1);
                break;
            case '2':
                _bits.set(pos + 2, 1);
                break;
            case '3':
                _bits.set(pos + 3, 1);
                _bits.set(pos + 2, 1);
                break;
            case '4':
                _bits.set(pos + 1, 1);
                break;
            case '5':
                _bits.set(pos + 1, 1);
                _bits.set(pos + 3, 1);
                break;
            case '6':
                _bits.set(pos + 1, 1);
                _bits.set(pos + 2, 1);
                break;
            case '7':
                _bits.set(pos + 1, 1);
                _bits.set(pos + 2, 1);
                _bits.set(pos + 3, 1);
                break;
            case '8':
                _bits.set(pos, 1);
                break;
            case '9':
                _bits.set(pos, 1);
                _bits.set(pos + 3, 1);
                break;
            case 'A':
                _bits.set(pos, 1);
                _bits.set(pos + 2, 1);
                break;
            case 'B':
                _bits.set(pos, 1);
                _bits.set(pos + 2, 1);
                _bits.set(pos + 3, 1);
                break;
            case 'C':
                _bits.set(pos, 1);
                _bits.set(pos + 1, 1);
                break;
            case 'D':
                _bits.set(pos, 1);
                _bits.set(pos + 1, 1);
                _bits.set(pos + 3, 1);
                break;
            case 'E':
                _bits.set(pos, 1);
                _bits.set(pos + 1, 1);
                _bits.set(pos + 2, 1);
                break;
            case 'F':
                _bits.set(pos, 1);
                _bits.set(pos + 1, 1);
                _bits.set(pos + 2, 1);
                _bits.set(pos + 3, 1);
                break;
            default: return false;
        }
        ++it;
    }
    _packet_size = it*4;
    return true;
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Transmission transmission;
    transmission.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX)) {
        if (!transmission.add_bits(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    const uint64_t answer2 = transmission.parse();
    const uint16_t answer1 = transmission.version_sum();

    file.close();
    transmission.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 16 completion time: %" PRIu64 "µs\n", completion_time);

    printf("Answer 1 = %u\n", answer1);
    printf("Answer 2 = %lu\n", answer2);

    return 0;
}
