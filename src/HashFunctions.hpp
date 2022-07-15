#ifndef HASHFUNCTIONS_INCLUDED
#define HASHFUNCTIONS_INCLUDED

#include <array>
#include <cstdint>
#include <cstddef>

class BasicHashFunction {
    private:
        static constexpr size_t keySize = 8;
        size_t numBits;
        std::array<std::array<uint64_t, 256>, keySize> shuffleBits;
    
    public:
        BasicHashFunction(size_t numBits);
        uint64_t getBits(uint64_t id);
        size_t operator() (uint64_t id);
        
};

class SimpleHashFunction {
    private:
        static constexpr size_t maxbits = 30;
        size_t numBits;
        size_t a, b;
    
    public:
        SimpleHashFunction(size_t numBits);
        uint64_t getBits(uint64_t id);
        size_t operator() (uint64_t id);
        
};

#endif
