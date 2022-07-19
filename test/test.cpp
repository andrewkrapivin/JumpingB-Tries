#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include <cassert>

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

    HashTable vEBTree{testSize*2ull};

    BenchHelper bench;

    for(size_t i{0}; i < testSize; i++) {
        vEBTree.insert(randomList[i]);
    }

    for(size_t i{0}; i < testSize; i++) {
        auto queryResult = vEBTree.pointQuery(randomList[i].key);
        assert(queryResult.has_value());
        assert(*queryResult == randomList[i].val);
    }
    
}

