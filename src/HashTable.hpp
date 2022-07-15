#ifndef HASHTABLE_INCLUDED
#define HASHTABLE_INCLUDED

#include "HashFunctions.hpp"
#include "FastBitset.hpp"
#include "ULLongByteString.hpp"
#include "vEBTypes.hpp"
#include <bitset>
#include <array>
#include <vector>
#include <optional>

//For now designing this hash table to be a Cuckoo Hash Table for 64-bit keys. Should be just one "cache miss" for point query and two for successor query

namespace vEB_BTree {
    struct alignas(64) HashBucket 
    {
        //Member refers to element in the table/tree that shares the prefix
        KeyValPair smallestMember;
        KeyValPair largestMember;
        //std::bitset<256> childMask; //Mask for each of the possible next bytes that "children" in the imaginary trie could have.
        // std::array<uint64_t, 4> childMask; //Wait lol why did I make this of type keyType? This should be uniform as uint64_t
        FastBitset<256> childMask;
        //To differentiate an empty bucket from one that holds key equal to zero, we require that childMask not be empty, since there is always either a successor or its the end and the values of childMask don't matter anyways.
    };
    
    static_assert(sizeof(HashBucket) == 64);
    
    static constexpr bool useLocks = false; //For now single threaded to make it easier

    //TODO: separate out the vEB part from the Cuckoo part?
    class HashTable {
        private:
            struct ModdedBasicHashFunction {
                size_t numBits;
                std::array<std::array<KeyType, 256>, KeySize> shuffleBits;
                ModdedBasicHashFunction(size_t numBits);
                size_t operator() (KeyType key);
            };
            std::array<ModdedBasicHashFunction, 2> hashFunctions;
            std::array<std::array<std::vector<HashBucket>, KeySize+1>, 2> tables; //Clearly this is inneficient because of several different things, like being very space inneficient, esp since the earlier layers don't use much, but well this approach is rather space inneficient anyways so whatever
            //But well at least compute the hash exactly once & don't need to worry about doing anything custom anymore
            bool testEntry(KeyType key, HashBucket entry, size_t depth); //tests if entry actually holds the key
            std::optional<HashBucket> successorEntry(HashBucket entry, size_t pos, size_t depth);            
        
        public:
            HashTable(size_t size);
            void insert(KeyValPair kvp);
            std::optional<ValType> pointQuery(KeyType key);
            KeyValPair successorQuery(KeyType key);
            KeyValPair predecessorQuery(KeyType key);
    };
}

#endif
