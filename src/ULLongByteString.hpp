#ifndef ULLongByteString_HPP
#define ULLongByteString_HPP

#include "vEBTypes.hpp"

namespace vEB_BTree {
    class ULLongByteString {
        private:
            ULLongType x;
            static constexpr ULLongType byteToBit(size_t byte) {return 8*byte;}
            static constexpr ULLongType getByteMask(size_t lowByte, size_t highByte) { //sets bits to one corresponding to byte range [lowByte, highByte)
                return (((ULLongType)1) << byteToBit(highByte)) - (((ULLongType)1ull) << byteToBit(lowByte));
            }

        public:
            ULLongByteString(uint64_t x): x(x) {}

            void maskOffBigBytes(size_t numBytes) {
                x &= - (((ULLongType)1ull) << byteToBit(numBytes));
            }
            void maskOffSmallBytes(size_t numBytes) {
                x &= (((ULLongType)1ull) << byteToBit(numBytes)) - 1;
            }

            operator uint64_t&() {return x;}
            operator uint64_t() const {return x;} //I need to understand this whole const stuff cause from what I understand its not even always respected like idk. Cause right now tbh it just seems like duplicated code for absolutely no reason

            ByteType getByte(size_t pos) {
                ULLongType mask = getByteMask(pos, pos+1);
                return (x & mask) >> byteToBit(pos);
            }

            void setByte(size_t pos, ByteType byte) {
                ULLongType mask = ~getByteMask(byte, byte+1);
                x = (x & mask) | (((ULLongType)byte) << byteToBit(pos));
            }

            // TODO: get some c++ practice by implementing this! Should make a reference class wrapper that then does the bit operations
            // some_reference& operator[](size_t byte);
            // const some_reference& operator[](size_t byte) const; //How does this even work its so weird
    };
}

#endif