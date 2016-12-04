#pragma once

#include <cstdint>
#include <cstring>

namespace btree {

// Record fulfil a function of key when it is an inner node.
// Record fulfil a function of value when it is a leaf.
union Record {
    uint64_t key;
    char value[8];
};

}