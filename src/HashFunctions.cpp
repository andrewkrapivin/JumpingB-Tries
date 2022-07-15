#include "HashFunctions.hpp"
#include <random>
#include <chrono>
#include <cassert>

BasicHashFunction::BasicHashFunction(size_t numBits): numBits{numBits} {
    //Since this hash function is crazy let's also go nuts with randomness. Have no idea whether rd actually has enough entropy for this to give any benefit, so whatever. Let's also add the clock to it then.
    std::random_device rd;
    std::seed_seq seed{rd(), rd(), rd(), rd(), rd()}; // std::chrono::steady_clock::now().time_since_epoch().count()
    std::mt19937 generator{seed};
    std::uniform_int_distribution<uint64_t> dist{0ull, (1ull << numBits) - 1};
    for(auto& b: shuffleBits) {
        for(size_t i{0}; i < 256; i++) {
            b[i] = dist(generator);
        }
    }
}

uint64_t BasicHashFunction::getBits(uint64_t id) {
    uint64_t res = 0;

    std::array<unsigned char, keySize> entriesToShuffle = std::bit_cast<std::array<unsigned char, keySize>, uint64_t>(id); //Wack this bit_cast thing is.
    for(size_t i{0}; i < keySize; i++) {
        res ^= shuffleBits[i][entriesToShuffle[i]];
    }
    
    return res;
}

size_t BasicHashFunction::operator() (uint64_t id) {
    return getBits(id);
}

SimpleHashFunction::SimpleHashFunction(size_t numBits): numBits{numBits} {
    assert(numBits <= maxbits);
    std::random_device rd;
    std::mt19937 generator{rd()};
    std::uniform_int_distribution<size_t> dist(0, (1ull << numBits) - 1);
    a = dist(generator);
    b = dist(generator);
    a = a*2 - 1;
    b = b*2 - 1;
}

uint64_t SimpleHashFunction::getBits(uint64_t id) {
    uint64_t lowerBits = id & ((1ull << 32) - 1);
    uint64_t higherBits = id >> 32;
    uint64_t hash = (lowerBits * a) ^ (higherBits*b);
    hash = hash & ((1ull << numBits) - 1);
    return hash;
}

size_t SimpleHashFunction::operator() (uint64_t id) {
    return getBits(id);
}

