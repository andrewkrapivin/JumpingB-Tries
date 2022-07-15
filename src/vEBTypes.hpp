#ifndef vEB_TYPES_HPP
#define vEB_TYPES_HPP

#include <cstdint>
#include <cstddef>
#include <climits>
#include <utility>

namespace vEB_BTree {
    using KeyType = uint64_t;
    using ValType = uint64_t;
    constexpr size_t KeySize = sizeof(KeyType);
    constexpr size_t KeySizeBits = KeySize * 8;
    using KeyValPair = std::pair<KeyType, ValType>;
    using ULLongType = uint64_t;
    using ByteType = uint8_t;
    constexpr size_t BitsInULLong = sizeof(ULLongType) * CHAR_BIT; //This is just being ridiculous but whatever. But technically uint64_t is not actually guaranteed to be 64 bits as I understand. Lol though in other places I am requiring that uint64_t be 64 bits but whatever
    //Even in this code I assume that!!! When using __builtin_ctzll/__builtin_clzll, lol.
}

#endif