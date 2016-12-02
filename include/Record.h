#pragma once

#include <cstdint>
#include <cstring>

namespace btree {

struct Record {
    Record() : key(0), value{0} {};
    Record(uint64_t _key, const char _value[8]) : key(_key) {
        std::memcpy(value, _value, sizeof(value));
    };

    uint64_t key;
    char value[8];
};

}