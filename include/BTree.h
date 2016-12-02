#pragma once

#include "Record.h"

#include <utility>
#include <cstdint>
#include <string>
#include <ostream>

namespace btree {

// Container methods' return/exceptions codes (success, errors...)
enum : int {
    SUCCESS = 0,
    BAD_PARAMETER = -1,
    NOT_IMPLEMENTED = -1000
};

class Container {
public:
    Container(uint32_t records_on_data_page, uint8_t records_in_index_node) : RECORDS_ON_DATA_PAGE(
            records_on_data_page), RECORDS_IN_INDEX_NODE(records_in_index_node) {
        if (records_in_index_node > 64) {
            throw int(BAD_PARAMETER);
        }
    };

    // Insert record into container.
    int insert(Record record);

    // Update record in container.
    // In record param put key of record you want to update and new value.
    int update(Record record);

    // Remove record from container.
    int remove(uint64_t key);

    // Get value of given key.
    int get_value(uint64_t key, std::string &value);

    // Print index file. Set raw to true, if you want to print whole raw file.
    int print_index(std::ostream out, bool raw = false);

    // Print data file. Set raw to true, if you want to print whole raw file.
    int print_data(std::ostream out, bool raw = false);

private:
    const uint32_t RECORDS_ON_DATA_PAGE;
    const uint8_t RECORDS_IN_INDEX_NODE;
};

}