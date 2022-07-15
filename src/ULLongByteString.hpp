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
            constexpr ULLongByteString(ULLongType x): x(x) {}

            void constexpr maskOffBigBytes(size_t numBytes) {
                x &= -(((ULLongType)1ull) << byteToBit(numBytes));
            }
            void constexpr maskOffSmallBytes(size_t numBytes) {
                x &= (((ULLongType)1ull) << byteToBit(numBytes)) - 1;
            }

            constexpr operator ULLongType&() {return x;}
            constexpr operator ULLongType() const {return x;} //I need to understand this whole const stuff cause from what I understand its not even always respected like idk. Cause right now tbh it just seems like duplicated code for absolutely no reason

            ByteType constexpr getByte(size_t pos) {
                ULLongType mask = getByteMask(pos, pos+1);
                return (x & mask) >> byteToBit(pos);
            }

            ULLongType constexpr getPrefix(size_t numBytes) {
                ULLongByteString y{x};
                y.maskOffBigBytes(numBytes);
                return y;
            }

            void constexpr setByte(size_t pos, ByteType byte) {
                ULLongType mask = ~getByteMask(byte, byte+1);
                x = (x & mask) | (((ULLongType)byte) << byteToBit(pos));
            }

            static constexpr ULLongType getPrefix(ULLongType x, size_t numBytes) {
                return ULLongByteString{x}.getPrefix(numBytes);
            }

            static constexpr bool comparePrefixes(ULLongType x, ULLongType y, size_t sizePrefix) {
                return getPrefix(x, sizePrefix) == getPrefix(y, sizePrefix);
            }

            // TODO: get some c++ practice by implementing this! Should make a reference class wrapper that then does the bit operations
            // some_reference& operator[](size_t byte);
            // const some_reference& operator[](size_t byte) const; //How does this even work its so weird
    };
}

#endif