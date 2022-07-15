#include <iostream>
#include <random>
#include <chrono>

#include "../src/HashTable.hpp"
#include "../src/BenchHelper.hpp"
#include "../src/ULLongByteString.hpp"

int main(int argc, char** argv){
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 generator (seed);

    BenchHelper bench;
     
    std::cout << "HEllo" << std::endl;
}

