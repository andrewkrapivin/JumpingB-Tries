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
            std::array<ModdedBasicHashFunction, 2> hashFunctions;
            std::array<std::array<std::vector<HashBucket>, KeySize+1>, 2> tables; //Clearly this is inneficient because of several different things, like being very space inneficient, esp since the earlier layers don't use much, but well this approach is rather space inneficient anyways so whatever
            //But well at least compute the hash exactly once & don't need to worry about doing anything custom anymore
            bool testEntry(const HashBucket& entry, KeyType key, size_t depth) const; //tests if entry actually holds the key
            std::optional<std::reference_wrapper<HashBucket>> loadDesiredEntry(KeyType key, size_t depth); //Here reference actually makes sense, but then I went ahead and made everything else a reference, which I guess is fine but just like why lol
            std::optional<std::reference_wrapper<HashBucket>> loadDesiredEntry(KeyType key, size_t depth, std::array<std::reference_wrapper<HashBucket>, 2> entries);
            std::optional<std::reference_wrapper<const HashBucket>> loadDesiredEntry(KeyType key, size_t depth) const;
            std::optional<std::reference_wrapper<const HashBucket>> loadDesiredEntry(KeyType key, size_t depth, std::array<std::reference_wrapper<const HashBucket>, 2> entries) const;
            std::optional<std::reference_wrapper<const HashBucket>> successorEntry(const HashBucket& entry, ByteType pos, size_t depth) const;
            std::vector<std::array<std::reference_wrapper<const HashBucket>, 2>> loadAllEntries(KeyType key) const; //really annoying that can't make this an array because it tries calling default constructor but there is no such thing there
            //Reply to that: wait why did I make these use references anyways? Why not just return a const HashBucket? Change this?
        
        public:
            HashTable(size_t size);
            void insert(KeyValPair kvp);
            std::optional<ValType> pointQuery(KeyType key) const;
            std::optional<KeyValPair> successorQuery(KeyType key) const;
            // std::optional<KeyValPair> predecessorQuery(KeyType key);
    };
}

#endif
