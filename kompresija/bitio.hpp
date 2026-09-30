#ifndef BITIO_HPP
#define BITIO_HPP

#include<string>
#include<stdexcept>

// Najveca duzina teksta koju kodiramo (duzina se upisuje na 32 bita).
const int MAX_SIZE = 1000000000;

// Broj bitova potreban da se zapise vrijednost iz [0, count).
inline int index_bits(int count) {
    int bits = 0;
    while((1 << bits) < count) {
        bits++;
    }
    return bits;
}

// Upis bitova redom od najznacajnijeg ka najmanje znacajnom u svakom bajtu.
class BitWriter {
public:
    BitWriter(): m_used(0) {}

    void writeBit(int bit) {
        if(m_used == 0) {
            m_bytes.push_back(0);
        }
        if(bit) {
            int last = m_bytes.size() - 1;
            m_bytes[last] = m_bytes[last] | (1 << (7 - m_used));
        }
        m_used = (m_used + 1) % 8;
    }

    void write(int value, int bits) {
        for(int i = bits - 1; i >= 0; i--) {
            writeBit((value >> i) & 1);
        }
    }

    const std::string& bytes() const {
        return m_bytes;
    }

private:
    std::string m_bytes;
    int m_used;
};

class BitReader {
public:
    BitReader(const std::string& bytes): m_bytes(bytes), m_pos(0) {}

    int readBit() {
        if(m_pos / 8 >= (long long)m_bytes.size()) {
            throw std::runtime_error("neocekivan kraj podataka");
        }
        int bit = (((unsigned char)m_bytes[m_pos / 8]) >> (7 - m_pos % 8)) & 1;
        m_pos++;
        return bit;
    }

    int read(int bits) {
        int value = 0;
        for(int i = 0; i < bits; i++) {
            value = value * 2 + readBit();
        }
        return value;
    }

private:
    const std::string& m_bytes;
    long long m_pos;
};

#endif
