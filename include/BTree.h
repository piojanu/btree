#pragma once

#include "Storage.h"
#include "Record.h"
#include "Node.h"

#include <cstdint>
#include <fstream>

namespace btree {

template <const uint32_t RECORDS_IN_INDEX_NODE>
class Container {
public:
    Container(std::string path) : Container() {
        file = new std::fstream();

        try {
            file->open(path, std::ios::in | std::ios::out | std::ios::trunc);
            if( file->good() ) {
                good = true;
            }
        }
        catch (...) {
            perror("Error occurred while opening file buffer in Container constructor!");
        }

        index_data = static_cast<std::iostream *>(file);
        storage = new Storage<RECORDS_IN_INDEX_NODE>(index_data, height);
    }

    Container(std::iostream *stream) : Container() {
        index_data = stream;
        storage = new Storage<RECORDS_IN_INDEX_NODE>(index_data, height);
    }

    ~Container() {
        if( file != nullptr ) {
            file->close();
            delete file;
        }

        delete storage;
    }

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

    // Returns true if object is properly initialised.
    bool is_good() {
        return good;
    }

private:
    Container() : storage(nullptr), index_data(nullptr), file(nullptr), height(1), good(false) {}

    Storage<RECORDS_IN_INDEX_NODE> *storage;
    std::iostream *index_data;
    std::fstream *file;
    uint64_t height;
    bool good;
};

}