#ifndef ULLongByteString_HPP
#define ULLongByteString_HPP

#include "vEBTypes.hpp"
#include <cassert>
#include <iostream>

namespace vEB_BTree {
    class ULLongByteString {
        private:
            ULLongType x;
            static constexpr ULLongType byteToBit(size_t byte) {return 8*(BytesInULLong-byte);}
            static ULLongType getByteMask(size_t lowByte, size_t highByte) { //sets bits to one corresponding to byte range [lowByte, highByte)
                return safeShiftLeft(((ULLongType)1ull), byteToBit(lowByte)) - safeShiftLeft(((ULLongType)1), byteToBit(highByte));
            }

        public:
            constexpr ULLongByteString(ULLongType x): x(x) {}

            //Keeps only the bytes [0, endByte) in order
            void constexpr keepBytesUntil(size_t endByte) {
                x &= -safeShiftLeft(((ULLongType)1ull), byteToBit(endByte));
            }

            //Keeps only the bytes [endByte, BytesInULLong)
            void constexpr keepBytesStartingWith(size_t startByte) {
                x &= safeShiftLeft((ULLongType)1ull, byteToBit(startByte)) - 1;
            }

            constexpr operator ULLongType&() {return x;}
            constexpr operator ULLongType() const {return x;} //I need to understand this whole const stuff cause from what I understand its not even always respected like idk. Cause right now tbh it just seems like duplicated code for absolutely no reason

            ByteType getByte(size_t pos) {
                ULLongType mask = getByteMask(pos, pos+1);
                // std::cout << std::hex << mask << std::endl;
                return safeShiftRight((x & mask), byteToBit(pos+1));
            }

            ULLongType constexpr getPrefix(size_t numBytes) {
                ULLongByteString y{x};
                y.keepBytesUntil(numBytes);
                return y;
            }

            void setByte(size_t pos, ByteType byte) {
                ULLongType mask = ~getByteMask(pos, pos+1);
                // std::cout << "t " << byteToBit(pos) << " " << byteToBit(pos+1) << std::endl;
                // std::cout << std::hex << pos << " " << (ULLongType(byte)) << " " << mask << std::endl;
                x = (x & mask) | safeShiftLeft(((ULLongType)byte), byteToBit(pos+1));
            }

            static constexpr ULLongType getPrefix(ULLongType x, size_t numBytes) {
                return ULLongByteString{x}.getPrefix(numBytes);
            }

            static constexpr bool comparePrefixes(ULLongType x, ULLongType y, size_t sizePrefix) {
                if(sizePrefix == 0) {
                    assert(getPrefix(x, sizePrefix) == getPrefix(y, sizePrefix) && getPrefix(y, sizePrefix) == 0);
                }
                return getPrefix(x, sizePrefix) == getPrefix(y, sizePrefix);
            }

            // TODO: get some c++ practice by implementing this! Should make a reference class wrapper that then does the bit operations
            // some_reference& operator[](size_t byte);
            // const some_reference& operator[](size_t byte) const; //How does this even work its so weird
    };
}

#endif