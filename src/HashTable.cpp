#include <random>
#include <chrono>
#include <ranges>
#include <algorithm>
#include <iostream>
#include <cassert>
#include "HashTable.hpp"

namespace vEB_BTree {
    HashBucket::HashBucket() {}
    HashBucket::HashBucket(KeyValPair kvp, size_t dep): smallestMember{kvp}, largestMember{kvp}, childMask{ULLongByteString{kvp.key}.getByte(dep)} {} //the last part works since it sets the right bit when dep < KeySize; otherwise we do not care which bit is set, just that some bit is set, and getByte would return 0, which is fine

    bool HashBucket::empty() {
        return childMask.empty();
    }

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
    
    size_t HashTable::ModdedBasicHashFunction::operator() (KeyType key) const {
        size_t res=0;

        std::array<unsigned char, KeySize> entriesToShuffle = std::bit_cast<std::array<unsigned char, KeySize>, KeyType>(key); //Wack this bit_cast thing is.
        for(size_t i{0}; i < KeySize; i++) {
            res ^= shuffleBits[i][entriesToShuffle[i]];
        }
        
        return res;
    }

    size_t HashTable::ModdedBasicHashFunction::operator() (KeyType key, size_t depth) const {
        size_t res=0;

        ULLongByteString keyString{key};
        for(size_t i{0}; i < depth; i++) {
            res ^= shuffleBits[i][keyString.getByte(i)];
        }
        
        return res;
    }

    HashTable::ModdedBasicHashFunction::Iterator::Iterator(const ModdedBasicHashFunction& hashFunction): hashFunction{hashFunction} {}

    size_t HashTable::ModdedBasicHashFunction::Iterator::Iterator::operator() (ByteType b) {
        curResult ^= hashFunction.shuffleBits[curDepth++][b];
        return curResult;
    }
    
    size_t HashTable::ModdedBasicHashFunction::Iterator::Iterator::operator() (KeyType key, size_t depthToSkipTo) {
        ULLongByteString keyString{key};
        for(; curDepth < depthToSkipTo; curDepth++) {
            curResult ^= hashFunction.shuffleBits[curDepth][keyString.getByte(curDepth)];
        }
        return curResult;
    }
    

    HashTable::HashTable(size_t size): numBits{64ull - __builtin_clzll(size)}, sizeTables{1ull << numBits}, hashFunctions{ModdedBasicHashFunction{numBits}, ModdedBasicHashFunction{numBits}} {
        for(auto& tablePair: tables) {
            for(auto& table: tablePair) {
                table = std::vector<HashBucket>(sizeTables, EmptyBucket); //Um why did vector not default initialize here?
            }
        }
    }

    bool HashTable::testEntry(const HashBucket& entry, KeyType key, size_t depth) const {
        return ULLongByteString::comparePrefixes(key, entry.smallestMember.key, depth) && !entry.childMask.empty();
    }

    std::array<HashBucket*, 2> HashTable::loadPossibleEntries(KeyType key, size_t depth) {
        std::array<size_t, 2> hashValues{hashFunctions[0](key, depth), hashFunctions[1](key, depth)};
        // std::array<std::reference_wrapper<HashBucket>, 2> possibleEntries{tables[0][depth][hashValues[0]], tables[1][depth][hashValues[1]]};
        return {&tables[0][depth][hashValues[0]], &tables[1][depth][hashValues[1]]};
    }

    HashBucket* HashTable::loadDesiredEntry(KeyType key, size_t depth) {
        return loadDesiredEntry(key, depth, loadPossibleEntries(key, depth));
    }

    HashBucket* HashTable::loadDesiredEntry(KeyType key, size_t depth, std::array<HashBucket*, 2> entries) {
        for(HashBucket* b: entries) {
            if(testEntry(*b, key, depth)) {
                return b;
            }
        }
        return NULL;
    }

    std::array<HashBucket, 2> HashTable::loadPossibleEntries(KeyType key, size_t depth) const {
        assert(depth<=KeySize);
        std::array<size_t, 2> hashValues{hashFunctions[0](key, depth), hashFunctions[1](key, depth)};
        // std::cout << hashValues[0] << " " << hashValues[1] << " " << key << " " << depth << std::endl;
        return {tables[0][depth][hashValues[0]], tables[1][depth][hashValues[1]]};
    }

    std::optional<HashBucket> HashTable::loadDesiredEntry(KeyType key, size_t depth) const {
        // std::array<size_t, 2> hashValues{hashFunctions[0](key, depth), hashFunctions[1](key, depth)};
        // std::array<std::reference_wrapper<const HashBucket>, 2> possibleEntries{tables[0][depth][hashValues[0]], tables[1][depth][hashValues[1]]};
        return loadDesiredEntry(key, depth, loadPossibleEntries(key, depth));
    }

    std::optional<HashBucket> HashTable::loadDesiredEntry(KeyType key, size_t depth, std::array<HashBucket, 2> entries) const {
        for(const HashBucket& b: entries) {
            if(testEntry(b, key, depth)) {
                return b;
            }
        }
        return {};
    }
    
    std::optional<HashBucket> HashTable::successorEntry(const HashBucket& entry, ByteType pos, size_t depth) const {
        if(depth == KeySize) return {};
        // std::cout << depth << std::endl;
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
        FastBitset<256> childMask = entry.childMask;
        childMask.clearSmallBits(pos); //Yeah maybe implement function in that to do these two steps in one step, which also maybe could be a bit more efficient, but whatever
        int successorByte = childMask.findSmallestBit();
        if(successorByte == -1) return {};
        ULLongByteString prefix{entry.smallestMember.key};
        prefix.setByte(depth, successorByte);
        std::optional<HashBucket> h = loadDesiredEntry(prefix, depth+1);
        assert(h.has_value());
        return h;
    }

    std::vector<std::array<HashBucket*, 2>> HashTable::loadAllEntries(KeyType key) {
        std::vector<std::array<HashBucket*, 2>> entries;
        std::array<ModdedBasicHashFunction::Iterator, 2> hashIterators{hashFunctions[0], hashFunctions[1]};
        ULLongByteString keyString{key};
        entries.push_back({&tables[0][0][0], &tables[1][0][0]});
        size_t dep = 1;
        for(; dep <= KeySize; dep++) {
            //Figure out something nicer to do with these arrays of size two
            size_t entryIndex1 = hashIterators[0](keyString.getByte(dep-1));
            size_t entryIndex2 = hashIterators[1](keyString.getByte(dep-1));
            // std::cout << "entryIndex1: " << entryIndex1 << ", 2: " << entryIndex2 << std::endl;
            assert(dep < tables[0].size() && entryIndex1 < tables[0][dep].size() && entryIndex2 < tables[1][dep].size());
            entries.push_back({&tables[0][dep][entryIndex1], &tables[1][dep][entryIndex2]});
            __builtin_prefetch(&tables[0][dep][entryIndex1]);
            __builtin_prefetch(&tables[1][dep][entryIndex2]);
            // for(size_t i{0}; i < 2; i++) {
            //     entries[dep][i] = tables[i][dep][hashIterators[i](keyString.getByte(dep))];
            // }
        }
        // for(size_t i{0}; i <= KeySize; i++) {
        //     std::cout << entries[i][0]-smallestMember.key << " " << entries[i][1].get().smallestMember.key << std::endl;
        // }
        return entries;
    }

    std::vector<std::array<HashBucket, 2>> HashTable::loadAllEntries(KeyType key) const {
        std::vector<std::array<HashBucket, 2>> entries;
        std::array<ModdedBasicHashFunction::Iterator, 2> hashIterators{hashFunctions[0], hashFunctions[1]};
        std::array<std::array<ULLongType, 2>, KeySize+1> indices;
        indices[0] = {0, 0};
        size_t dep = 1;
        ULLongByteString keyString{key};
        for(; dep <= KeySize; dep++) {
            indices[dep] = {hashIterators[0](keyString.getByte(dep-1)), hashIterators[1](keyString.getByte(dep-1))};
        }
        // for(dep = 0; dep <= KeySize; dep++) {
        //     __builtin_prefetch(&tables[0][dep][indices[dep][0]]);
        //     __builtin_prefetch(&tables[1][dep][indices[dep][1]]);
        // }
        for(dep = 0; dep <= KeySize; dep++) {
            //Figure out something nicer to do with these arrays of size two
            entries.push_back({tables[0][dep][indices[dep][0]], tables[1][dep][indices[dep][1]]});
            // for(size_t i{0}; i < 2; i++) {
            //     entries[dep][i] = tables[i][dep][hashIterators[i](keyString.getByte(dep))];
            // }
        }
        return entries;
    }

    bool HashTable::hasKeyAsChild(const HashBucket& entry, KeyType key, size_t depth) const {
        //Note: maybe having this ULLongByteString type is actually inconvenient. Like just make all the member functions static. It is convenient if we set KeyType to ULLongByteString, so could do that too
        ULLongByteString keyString{key};
        ByteType childByte = keyString.getByte(depth);
        return entry.childMask.getBit(childByte);
    }

    void HashTable::cuckooInsertEntry(KeyValPair kvp, size_t depth) {
        // std::cout << "Creating cuckoo entry, depth: " << depth << std::endl;
        HashBucket entry{kvp, depth};
        assert(!entry.childMask.empty());
        size_t parity = 0;
        // HashBucket& entryToInsert = tables[parity][depth][hashFunctions[parity](entry.smallestMember.key, depth)];
        // if(depth == 0) {
        //     assert(hashFunctions[parity](entry.smallestMember.key, depth) == 0);
        // }
        // std::swap(entry, entryToInsert); //I think this works to swap the value that entry holds and entryToInsert holds, even though one is a ref and one is not
        // while(!entry.childMask.empty()) {
        //     std::cout << "NANDE SUKA" << std::endl;
        //     assert(!entryToInsert.childMask.empty());
        //     if(depth == 0) {
        //         assert(!tables[0][0][0].childMask.empty());
        //         return;
        //     }
        //     parity ^= 1;
        //     entryToInsert = tables[parity][depth][hashFunctions[parity](entry.smallestMember.key, depth)];
        //     std::swap(entry, entryToInsert);
        // }
        do {
            HashBucket& entryToInsert = tables[parity][depth][hashFunctions[parity](entry.smallestMember.key, depth)];
            // std::cout << hashFunctions[parity](entry.smallestMember.key, depth) << " " << depth << " " << ((ULLongByteString)entry.smallestMember.key).getPrefix(depth) << " " << ((ULLongType)((ULLongByteString)entry.smallestMember.key).getByte(depth-1)) << std::endl;
            std::swap(entry, entryToInsert);
            parity ^= 1;
        } while(!entry.empty());
    }

    void HashTable::insertKeyToEntry(HashBucket* entry, KeyValPair kvp, size_t depth) {
        entry->smallestMember.setIfMin(kvp);
        entry->largestMember.setIfMax(kvp);
        entry->childMask.setBit(ULLongByteString(kvp.key).getByte(depth));
    }

    size_t count = 0;
    void HashTable::insert(KeyValPair kvp) {
        std::vector<std::array<HashBucket*, 2>> entries = loadAllEntries(kvp.key);
        assert(entries.size() == KeySize+1);
        // std::vector<HashBucket> correctEntries;
        size_t dep = 0;
        for(auto& entryPair: entries) {
            // std::cout << entryPair[0].get().smallestMember.key << " " << entryPair[1].get().smallestMember.key << std::endl;
            HashBucket* correctEntry = loadDesiredEntry(kvp.key, dep, entryPair);
            if(correctEntry == NULL) {
                assert(dep > 0 || count == 0);
                cuckooInsertEntry(kvp, dep);
                // if(dep == 0) {
                //     std::cout << tables[0][0][0].childMask.numBitsSet() << std::endl;
                // }
            }
            else {
                insertKeyToEntry(correctEntry, kvp, dep);
            }
            dep++;
        }
        count ++;
    }
    
    std::optional<ValType> HashTable::pointQuery(KeyType key) const {
        // std::array<HashBucket, 2> entries{tables[0][KeySize][hashFunctions[0](key)], tables[1][KeySize][hashFunctions[1](key)]};
        // for(HashBucket entry: entries) {
        //     //For the final "layer" so to say, we demand that there is exactly one key, so let's just say set only smallestMember
        //     if (testEntry(entry, key, 8)) {
        //         return entry.smallestMember.val;
        //     }
        // }
        // std::optional<HashBucket> entry = loadDesiredEntry(key, KeySize);
        // if(entry.has_value()) {
        //     return entry->smallestMember.val;
        // }
        // return {};
        const auto& correctEntry = loadDesiredEntry(key, KeySize);
        if(correctEntry.has_value()) {
            return correctEntry->smallestMember.val;
        }
        return {};
        // auto entries = loadAllEntries(key);
        // std::vector<HashBucket> correctEntries;
        // size_t dep = 0;
        // for(const auto& entryPair: entries) {
        //     const auto correctEntry = loadDesiredEntry(key, dep, entryPair);
        //     if(correctEntry.has_value()) {
        //         if(correctEntry->get().smallestMember.key == key && correctEntry->get().largestMember.key == key) { //Can go through this in reverse order to make this if statement check just whether correctEntry.smallestMember.key == key but whatever for now
        //             return correctEntry->get().smallestMember.val;
        //         }
        //     }
        // }
        // return {};
    }
    
    std::optional<KeyValPair> HashTable::successorQuery(KeyType key) {
        // std::array<std::array<HashBucket, 2>, KeySize+1> entries; //Fix order in hash table too cause here its opposite. Doesn't really matter there but whatever
        // std::array<unsigned char, KeySize> entriesToShuffle = std::bit_cast<std::array<unsigned char, KeySize>, KeyType>(key); //Wack this bit_cast thing is.
        // //And fix order in this for loop.
        // //This should be populated by the hash function?
        // for(size_t i{0}; i < 2; i++) {
        //     ModdedBasicHashFunction& h = hashFunctions[0];
        //     uint64_t res = 0;
        //     entries[0][i] = tables[i][0][0];
        //     for(size_t j{1}; j < KeySize+1; j++) {
        //         res ^= h.shuffleBits[j-1][entriesToShuffle[j-1]];
        //         entries[j][i] = tables[i][j][res];
        //     }
        // }
        
        // //Oh wow this function is kinda wrong. Since you don't know kind of at what stage the successor is (cause maybe up to some of the bits that match only people smaller than you exist), there is no point in actually figuring out where the first difference is.
        // //We just need to preload the successor HashBucket for every single HashTable (that matches us).
        // HashBucket pBucket;
        // for(size_t i{0}; i < KeySize+1; i++) {
            
        //     bool foundFirstDifference = true;
        //     for(size_t j{0}; j < 2; j++) {
        //         HashBucket& entry = entries[i][j];
        //         // if (keycmp == entry.smallestMember.first & mask) { //when i = 0 this is guaranteed to succeed. Honestly should make that more clear/program it a bit better
        //         //     foundFirstDifference = false;
        //         //     pBucket = entry;
        //         // }
        //     }
        //     if(foundFirstDifference) {
        //         //we now go "up" and then "down" a different path
                
        //     }
        // }

        auto entries = loadAllEntries(key);
        std::vector<HashBucket*> correctEntries;
        size_t dep = 0;
        for(const auto& entryPair: entries) {
            if(dep == KeySize) { break; }
            const auto correctEntry = loadDesiredEntry(key, dep, entryPair);
            if(correctEntry != NULL) {
                correctEntries.push_back(correctEntry);
            }
            else {
                break;
            }
            dep++;
        }

        dep = correctEntries.size()-1;
        //Keeping the value of dep here is a bit sus.
        ULLongByteString keyString{key};
        for(auto entry: correctEntries | std::views::reverse) {
            auto successor = successorEntry(*entry, keyString.getByte(dep), dep);
            if(successor.has_value()) {
                return successor->smallestMember;
            }
            dep--;
        }

        return {};
    }
};
