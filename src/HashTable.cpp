#include "HashTable.hpp"
#include <random>
#include <chrono>

namespace vEB_BTree {
    HashTable::ModdedBasicHashFunction::ModdedBasicHashFunction(size_t numBits): numBits{numBits} {
        std::random_device rd;
        std::seed_seq seed{rd(), rd(), rd(), rd(), rd()};
        std::mt19937 generator{seed};
        std::uniform_int_distribution<KeyType> dist{0ull, (1ull << numBits) - 1};
        for(auto& b: shuffleBits) {
            for(size_t i{0}; i < 256; i++) {
                b[i] = dist(generator);
            }
        }
    }
    
    size_t HashTable::ModdedBasicHashFunction::operator() (KeyType key) {
        size_t res= 0;

        std::array<unsigned char, KeySize> entriesToShuffle = std::bit_cast<std::array<unsigned char, KeySize>, KeyType>(key); //Wack this bit_cast thing is.
        for(size_t i{0}; i < KeySize; i++) {
            res ^= shuffleBits[i][entriesToShuffle[i]];
        }
        
        return res;
    }
    
    bool HashTable::testEntry(KeyType key, HashBucket entry, size_t depth) {
        KeyType mask = (1ull << (8*depth)) - 1;
        KeyType keycmp = key & (~mask);
        if (keycmp == (entry.smallestMember.first & mask)) {
            return !entry.childMask.empty(); 
        }
        return false;
    }
    
    std::optional<HashBucket> HashTable::successorEntry(HashBucket entry, size_t pos, size_t depth) {
        // size_t index = pos/64; //maybe move these constants out somewhere
        // uint64_t offset = pos & 63;
        // entry.childMask[index] &= ~(offset - 1);
        // uint64_t desiredPos = -1ull; //should be uint8_t but idk why just for compat or smth made it this
        // for(size_t j{index}; j < entry.childMask.size(); j++) {
        //     if(childMask[index] == 0) continue;
        //     desiredPos = _tzcnt_u64(childMask[index]) + j*KeySizeBits;
        //     break;
        // }
        // if(desiredPos == -1ull) return {};
        // //Now get the two possible entries, compare the prefix and the desiredPos together to see which one matches, and then return.
        // KeyType mask = (1ull << (8*depth)) - 1;
        // KeyType keycmp = key & (~mask);
        // KeyType desiredPrefix = keycmp + (desiredPos << (8*(KeySize-depth));
        return {};
    }
    
    std::optional<ValType> HashTable::pointQuery(KeyType key) {
        std::array<HashBucket, 2> entries{tables[0][KeySize][hashFunctions[0](key)], tables[1][KeySize][hashFunctions[1](key)]};
        for(HashBucket entry: entries) {
            //if (entry.smallestMember.first == key) { //For the final "layer" so to say, we demand that there is exactly one key, so let's just say set only smallestMember
            if (testEntry(key, entry, 8)) {
                return entry.smallestMember.second;
            }
        }
        return {};
    }
    
    KeyValPair HashTable::successorQuery(KeyType key) {
        std::array<std::array<HashBucket, 2>, KeySize+1> entries; //Fix order in hash table too cause here its opposite. Doesn't really matter there but whatever
        std::array<unsigned char, KeySize> entriesToShuffle = std::bit_cast<std::array<unsigned char, KeySize>, KeyType>(key); //Wack this bit_cast thing is.
        //And fix order in this for loop.
        //This should be populated by the hash function?
        for(size_t i{0}; i < 2; i++) {
            ModdedBasicHashFunction& h = hashFunctions[0];
            uint64_t res = 0;
            entries[0][i] = tables[i][0][0];
            for(size_t j{1}; j < KeySize+1; j++) {
                res ^= h.shuffleBits[j-1][entriesToShuffle[j-1]];
                entries[j][i] = tables[i][j][res];
            }
        }
        
        //Oh wow this function is kinda wrong. Since you don't know kind of at what stage the successor is (cause maybe up to some of the bits that match only people smaller than you exist), there is no point in actually figuring out where the first difference is.
        //We just need to preload the successor HashBucket for every single HashTable (that matches us).
        HashBucket pBucket;
        for(size_t i{0}; i < KeySize+1; i++) {
            
            bool foundFirstDifference = true;
            for(size_t j{0}; j < 2; j++) {
                HashBucket& entry = entries[i][j];
                // if (keycmp == entry.smallestMember.first & mask) { //when i = 0 this is guaranteed to succeed. Honestly should make that more clear/program it a bit better
                //     foundFirstDifference = false;
                //     pBucket = entry;
                // }
            }
            if(foundFirstDifference) {
                //we now go "up" and then "down" a different path
                
            }
        }
    }
};
