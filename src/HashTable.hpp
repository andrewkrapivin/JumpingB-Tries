#ifndef HASHTABLE_INCLUDED_HPP
#define HASHTABLE_INCLUDED_HPP

#include <bitset>
#include <array>
#include <vector>
#include <optional>
#include <functional>
#include "HashFunctions.hpp"
#include "FastBitset.hpp"
#include "ULLongByteString.hpp"
#include "vEBTypes.hpp"
#include "HugePageAllocator.hpp"

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
        HashBucket();
        HashBucket(KeyValPair kvp, size_t dep);

        bool empty();
    };

    const HashBucket EmptyBucket{};
    
    static_assert(sizeof(HashBucket) == 64);
    
    static constexpr bool useLocks = false; //For now single threaded to make it easier

    //TODO: separate out the vEB part from the Cuckoo part?
    class HashTable {
        private:
            struct ModdedBasicHashFunction {
                size_t numBits;
                std::array<std::array<KeyType, 256>, KeySize> shuffleBits;
                ModdedBasicHashFunction(size_t numBits);
                size_t operator() (KeyType key) const;
                size_t operator() (KeyType key, size_t depth) const;
                //Todo: make the iterator a real iterator
                struct Iterator { //TODO: implement this
                    size_t curDepth{0};
                    size_t curResult{0};
                    const ModdedBasicHashFunction& hashFunction;
                    Iterator(const ModdedBasicHashFunction&);
                    size_t operator() (ByteType b);
                    size_t operator() (KeyType key, size_t depthToSkipTo);
                };
            };

            //TODO: not redo computation? Cause there's a lot of functions assuming that input is not properly masked and so multiple instances of the same masking. For now whatever but for the future.
            size_t numBits;
            size_t sizeTables;
            std::array<ModdedBasicHashFunction, 2> hashFunctions;
            std::array<std::array<std::vector<HashBucket, thp_allocator<HashBucket>>, KeySize+1>, 2> tables; //Clearly this is inneficient because of several different things, like being very space inneficient, esp since the earlier layers don't use much, but well this approach is rather space inneficient anyways so whatever
            //But well at least compute the hash exactly once & don't need to worry about doing anything custom anymore
            bool testEntry(const HashBucket& entry, KeyType key, size_t depth) const; //tests if entry actually holds the key
            std::array<HashBucket*, 2> loadPossibleEntries(KeyType key, size_t depth);
            HashBucket* loadDesiredEntry(KeyType key, size_t depth); //Here reference actually makes sense, but then I went ahead and made everything else a reference, which I guess is fine but just like why lol
            HashBucket* loadDesiredEntry(KeyType key, size_t depth, std::array<HashBucket*, 2> entries);
            std::array<HashBucket, 2> loadPossibleEntries(KeyType key, size_t depth) const;
            std::optional<HashBucket> loadDesiredEntry(KeyType key, size_t depth) const;
            std::optional<HashBucket> loadDesiredEntry(KeyType key, size_t depth, std::array<HashBucket, 2> entries) const;
            std::vector<std::array<HashBucket*, 2>> loadAllEntries(KeyType key);
            std::vector<std::array<HashBucket, 2>> loadAllEntries(KeyType key) const; //really annoying that can't make this an array because it tries calling default constructor but there is no such thing there
            std::optional<HashBucket> successorEntry(const HashBucket& entry, ByteType pos, size_t depth) const;
            //Assumes that the prefix up till now matches, then asks if the next byte is present in the childmask. The name is somewhat bad I think
            bool hasKeyAsChild(const HashBucket& entry, KeyType key, size_t depth) const;
            //Reply to that: wait why did I make these use references anyways? Why not just return a const HashBucket? Change this?
            void cuckooInsertEntry(KeyValPair kvp, size_t depth);
            void insertKeyToEntry(HashBucket* entry, KeyValPair kvp, size_t depth);
        
        public:
            HashTable(size_t size);
            //Maybe make insert tell you if the key was already there & also have an option to say whether you want to overwrite existing value
            void insert(KeyValPair kvp);
            void deleteKey(KeyType key);
            std::optional<ValType> pointQuery(KeyType key) const;
            std::optional<KeyValPair> successorQuery(KeyType key);
            // std::optional<KeyValPair> predecessorQuery(KeyType key);
    };
}

#endif
