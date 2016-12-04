#pragma once

#include "Record.h"
#include "Node.h"

#include <cstdint>
#include <fstream>

namespace btree {

// Container methods' return/exceptions codes (success, errors...)
enum : int {
    SUCCESS = 0,
    RECORD_NOT_FOUND = -1,
    NOT_IMPLEMENTED = -1000
};

template <const uint32_t RECORDS_IN_INDEX_NODE>
class Container {
public:
    // Insert record into container.
    int insert(uint64_t key, const char value[8]) {
        return NOT_IMPLEMENTED;
    }

    // Assign new value for record with given key.
    int update(uint64_t key, const char value[8]) {
        return NOT_IMPLEMENTED;
    }

    // Remove record from container.
    int remove(uint64_t key) {
        return NOT_IMPLEMENTED;
    }

    // Get value of given key.
    int get_value(uint64_t key, const char value[8]) {
        return NOT_IMPLEMENTED;
    }

    // Print all data in order.
    int print_data_ordered(std::ostream &out) {
        return NOT_IMPLEMENTED;
    }

    // Print raw index file.
    int print_raw_file(std::ostream &out) {
        return NOT_IMPLEMENTED;
    }

private:
    std::fstream file;
};

}