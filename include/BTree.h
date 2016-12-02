#pragma once

#include "Record.h"
#include "Node.h"

#include <utility>
#include <cstdint>
#include <string>
#include <ostream>

namespace btree {

// Container methods' return/exceptions codes (success, errors...)
enum : int {
    SUCCESS = 0,
    NOT_IMPLEMENTED = -1000
};

template <const uint32_t RECORDS_ON_DATA_PAGE, const uint8_t RECORDS_IN_INDEX_NODE,
        typename = typename std::enable_if<RECORDS_IN_INDEX_NODE <= 64>::type>
class Container {
public:
    // Insert record into container.
    int insert(Record record) {
        return NOT_IMPLEMENTED;
    }

    // Update record in container.
    // In record param put key of record you want to update and new value.
    int update(Record record) {
        return NOT_IMPLEMENTED;
    }

    // Remove record from container.
    int remove(uint64_t key) {
        return NOT_IMPLEMENTED;
    }

    // Get value of given key.
    int get_value(uint64_t key, std::string &value) {
        return NOT_IMPLEMENTED;
    }

    // Print index file. Set raw to true, if you want to print whole raw file.
    int print_raw_index(std::ostream &out) {
        return NOT_IMPLEMENTED;
    }

    // Print data file. Set raw to true, if you want to print whole raw file.
    int print_raw_data(std::ostream &out) {
        return NOT_IMPLEMENTED;
    }
};

}