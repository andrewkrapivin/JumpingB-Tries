#include "vEBTypes.hpp"

namespace vEB_BTree {
    // KeyType getKeyFromKVP(KeyValPair kvp) {return kvp.first;}
    // ValType getValFromKVP(KeyValPair kvp) {return kvp.second;}
    // KeyValPair makeKVP(KeyType key, ValType val) {return {key, val};}

    KeyValPair::KeyValPair(KeyType key, ValType val): key(key), val(val) {}
    
    void KeyValPair::setIfMin(KeyValPair kvp) {
        if(kvp.key <= key) {
            key = kvp.key;
            val = kvp.val;
        }
    }

    void KeyValPair::setIfMax(KeyValPair kvp) {
        if(kvp.key >= key) {
            key = kvp.key;
            val = kvp.val;
        }
    }

    bool KeyValPair::operator< (const KeyValPair& rhs) const {
        if(key == rhs.key) {
            return val < rhs.val;
        }
        return key < rhs.key;
    }
}