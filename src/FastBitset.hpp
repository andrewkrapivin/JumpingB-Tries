#ifndef FAST_BITSET_HPP
#define FAST_BITSET_HPP

#include <array>
#include <ranges>
#include "vEBTypes.hpp"

//Sidenote: why are c++ unions terrible? Why is it not possible to just use it as a form of simple effectively bit casting for two different expressions of the same underlying data? Why the ridiculous and vague active member nonsense?
//I want to just represent these bits as either longs or chars, for example, or as bitset since sometimes that is nice but not supporting parallel operations

namespace vEB_BTree {
    template<size_t NumBits>
    class FastBitset {
        private:
            static constexpr size_t NumULLongs = (NumBits+BitsInULLong-1) / BitsInULLong; //check this is diff for diff NumBits
            std::array<ULLongType, NumULLongs> bits;

        public:
            FastBitset();
            void clearBigBits(size_t startBit); //clears bits [startBit, NumBits). Not sure if should include startBit, and its a bit easier to code including it so for now that's how it is
            void clearSmallBits(size_t startBit); //clears bits [0, startBit]
            int findSmallestBit() const; //essentially extended _tzcnt_u64. Returns location of the smallest bit
            int findLargestBit() const; //essentially extended _lzcnt_u64 except also with arithmetic to tell you which bit in big endian order
            bool empty() const;
            void setBit(size_t bit);
            void clearBit(size_t bit);

    };

    template<size_t NumBits>
    FastBitset<NumBits>::FastBitset() {}
    
    template<size_t NumBits>
    void FastBitset<NumBits>::clearBigBits(size_t startBit) {
        size_t startULLong = startBit / BitsInULLong;
        size_t bitOffset = startBit % BitsInULLong; //compiler should optimize % away probably
        ULLongType mask = (((ULLongType)1) << bitOffset) - 1; //only the lower bits are kept, so keep those at one
        bits[startULLong] &= mask;
        for(size_t i{startULLong+1}; i < bits.size(); i++) {
            bits[i] = 0;
        }
    }

    template<size_t NumBits>
    void FastBitset<NumBits>::clearSmallBits(size_t startBit) {
        size_t startULLong = startBit / BitsInULLong;
        size_t bitOffset = startBit % BitsInULLong; //compiler should optimize % away probably
        ULLongType mask = -(((ULLongType)1) << bitOffset); //only the higher bits are kept
        bits[startULLong] &= mask;
        for(size_t i{startULLong-1}; i > 0; i--) {
            bits[i] = 0;
        }
    }

    //Just implement it in simple bit scan version? Cause not being the most efficient really with this anyways
    template<size_t NumBits>
    int FastBitset<NumBits>::findSmallestBit() const {
        uint64_t offset{0};
        for(ULLongType l: bits) {
            if (l != 0) {
                static_assert(sizeof(unsigned long long) == sizeof(uint64_t)); //whatever
                return offset + __builtin_ctzll(l);
            }
            offset += BitsInULLong;
        }

        return -1;
    }
    
    template<size_t NumBits>
    int FastBitset<NumBits>::findLargestBit() const {
        uint64_t offset{NumULLongs*BitsInULLong-1};
        for(ULLongType l: bits | std::views::reverse) {
            if (l != 0) {
                static_assert(sizeof(unsigned long long) == sizeof(uint64_t)); //whatever
                return offset - __builtin_clzll(l);
            }
            offset -= BitsInULLong;
        }
        
        return -1;
    }

    template<size_t NumBits>
    bool FastBitset<NumBits>::empty() const {
        bool notEmpty = false;
        for(ULLongType l: bits) {
            notEmpty = notEmpty || (l != 0);
        }
        return !notEmpty;
    }

    template<size_t NumBits>
    void FastBitset<NumBits>::setBit(size_t bit) {
        size_t posULLong = bit / BitsInULLong;
        size_t bitOffset = bit % BitsInULLong;
        bits[posULLong] |= ((ULLongType)1) << bitOffset;
    }

    template<size_t NumBits>
    void FastBitset<NumBits>::clearBit(size_t bit) {
        size_t posULLong = bit / BitsInULLong;
        size_t bitOffset = bit % BitsInULLong;
        bits[posULLong] &= ~(((ULLongType)1) << bitOffset);
    }
}

#endif