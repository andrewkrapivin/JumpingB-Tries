#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include <cassert>
#include <algorithm>

#include "../src/HashTable.hpp"
#include "../src/BenchHelper.hpp"
#include "../src/ULLongByteString.hpp"

using namespace std;
using namespace vEB_BTree;

int main(int argc, char** argv){
    unsigned seed = chrono::steady_clock::now().time_since_epoch().count();
    mt19937 generator (seed);

    size_t testSize = 10;
    if(argc >= 2)
        testSize = atoi(argv[1]);
    
    vector<KeyValPair> randomList;
    uniform_int_distribution<uint64_t> dist(0, ULLONG_MAX);
    for(size_t i{0}; i < testSize; i++) {
        randomList.emplace_back(dist(generator), dist(generator));
    }
    shuffle(randomList.begin(), randomList.end(), generator);

    // ULLongByteString x{0x123456789abcdef0};
    // cout << hex << x.getPrefix(2) << endl;
    // cout << hex << ((ULLongType)x.getByte(0)) << endl;

    HashTable vEBTree{testSize*4ull/3};

    BenchHelper bench;

    bench.timeFunction([&] ()-> void {
        for(size_t i{0}; i < testSize; i++) {
            vEBTree.insert(randomList[i]);
        }
    }, "insert");

    bench.timeFunction([&] ()-> void {
        for(size_t i{0}; i < testSize; i++) {
            auto queryResult = vEBTree.pointQuery(randomList[i].key);
            assert(queryResult.has_value());
            assert(*queryResult == randomList[i].val);
        }
    }, "point query");

    sort(randomList.begin(), randomList.end());

    // for(size_t i{0}; i < 100; i++) {
        bench.timeFunction([&] ()-> void {
            for(size_t i{0}; i < testSize-1; i++) {
                auto queryResult = vEBTree.successorQuery(randomList[i].key);
                assert(queryResult.has_value());
                // cout << "hh: " << randomList[i].key << ' ' << queryResult->key << ' ' << randomList[i+1].key << endl;
                assert(queryResult->key == randomList[i+1].key);
            }
        }, "successor query");
    // }
    
}

