#ifndef vEB_TYPES_HPP
#define vEB_TYPES_HPP

#include <cstdint>
#include <cstddef>
#include <climits>
#include <utility>

namespace vEB_BTree {
    using ULLongType = uint64_t;
    using KeyType = ULLongType;
    using ValType = ULLongType;
    constexpr size_t KeySize = sizeof(KeyType);
    constexpr size_t KeySizeBits = KeySize * 8;
    // using KeyValPair = std::pair<KeyType, ValType>;
    struct KeyValPair {
        KeyType key;
        ValType val;

        KeyValPair(KeyType key=0, ValType val=0);

    };
    // KeyType getKeyFromKVP(KeyValPair kvp); //Interesting. So functions CANNOT have definitions in header file? But that is necessary templates and works with class functions (which how are those different from ordinary functions in this namespace? Cause that doesn't seem to make sense. Cause I have header guaard here, but it doesn't seem to work for some reason?
    // ValType getValFromKVP(KeyValPair kvp);
    // KeyValPair makeKVP(KeyType key, ValType val);
    using ByteType = uint8_t;
    constexpr size_t BitsInULLong = sizeof(ULLongType) * CHAR_BIT; //This is just being ridiculous but whatever. But technically uint64_t is not actually guaranteed to be 64 bits as I understand. Lol though in other places I am requiring that uint64_t be 64 bits but whatever
    //Even in this code I assume that!!! When using __builtin_ctzll/__builtin_clzll, lol.
}

#endif