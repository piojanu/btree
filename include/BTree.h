#pragma once

#include <utility>
#include <cstdint>
#include <string>
#include <ostream>

namespace btree {

// BTree record type.
typedef std::pair<uint64_t, std::string> record;

class Container {
public:
    // PAGE_SIZE is number of records per node.
    Container(uint32_t page_size) : PAGE_SIZE(page_size) {};

    // Insert record into container.
    int insert(record rec);

    // Update record in container.
    // In record param put key of record you want to update and new value.
    int update(record rec);

    // Remove record from container.
    int remove(uint64_t key);

    // Get value of given key.
    int get_value(uint64_t key, std::string &value);

    // Print index file. Set raw to true, if you want to print whole raw file.
    int print_index(std::ostream out, bool raw = false);

    // Print data file. Set raw to true, if you want to print whole raw file.
    int print_data(std::ostream out, bool raw = false);

private:
    // PAGE_SIZE is number of records per node.
    const uint32_t PAGE_SIZE;
};

// Return value codes (success, errors...)
enum {
    SUCCESS = 0,
    NOT_IMPLEMENTED = -1000
};

}