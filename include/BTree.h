#pragma once

#include "Storage.h"
#include "Record.h"
#include "Node.h"

#include <cstdint>
#include <fstream>

namespace btree {

template<const uint32_t RECORDS_IN_NODE>
class Container {
public:
    Container(std::string path) : Container() {
        file = new std::fstream();

        try {
            file->open(path, std::ios::in | std::ios::out | std::ios::trunc);
            if (file->good()) {
                good = true;
            }
        }
        catch (...) {
            perror("Error occurred while opening file buffer in Container constructor!");
        }

        index_data = static_cast<std::iostream *>(file);
        storage = new Storage<RECORDS_IN_NODE>(index_data, height);
    }

    Container(std::iostream *stream, const uint64_t &_height, const uint64_t &nodes_count) : Container() {
        index_data = stream;
        height = _height;
        storage = new Storage<RECORDS_IN_NODE>(index_data, height, nodes_count);
    }

    ~Container() {
        if (file != nullptr) {
            file->close();
            delete file;
        }

        delete storage;
    }

    // Insert record into container.
    int insert(uint64_t key, const char value[8]) {
        if (height == 0) { // Empty tree
            auto root_leaf = Node<RECORDS_IN_NODE>();
            root_leaf.usage = 1;
            root_leaf.node_entries[0].offset = key;
            strcpy(root_leaf.node_entries[0].record.value, value);

            auto ret = storage->write_node(0, &root_leaf);
            height = 1;

            return ret;
        } else { // Tree with at least root
            auto ext_node = new ExtendedNode<RECORDS_IN_NODE>[height];
            auto leaf = ext_node + height - 1;

            auto ret = storage->find_node(key, ext_node);
            if (ret != SUCCESS) {
                goto ERROR_HANDLER;
            }

            if (leaf->node.node_entries[leaf->index].offset == key) {
                ret = RECORD_EXISTS;
                goto ERROR_HANDLER;
            }

            if (leaf->node.usage < RECORDS_IN_NODE) { // Leaf has space for new record.
                // Make place for new record. Shift higher values to right.
                for (int64_t i = leaf->node.usage - 1; i >= leaf->index; i--) {
                    memcpy(&leaf->node.node_entries[i + 1], &leaf->node.node_entries[i], sizeof(NodeEntry));
                }

                leaf->node.node_entries[leaf->index].offset = key;
                strcpy(leaf->node.node_entries[leaf->index].record.value, value);
                leaf->node.usage++;

                auto ret = storage->write_node(leaf->ptr, &leaf->node, true);
            } else { // Leaf doesn't have space for new record.
                ret = NOT_IMPLEMENTED;
                goto ERROR_HANDLER;
            }

ERROR_HANDLER:
            delete[] ext_node;
            return ret;
        }
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
    int get_value(uint64_t key, char value[8]) {
        auto ext_node = new ExtendedNode<RECORDS_IN_NODE>[height];
        auto leaf = ext_node + height - 1;

        auto ret = storage->find_node(key, ext_node);
        if (ret != SUCCESS) {
            goto ERROR_HANDLER;
        }

        // Check if record found
        if (ext_node[height - 1].index < RECORDS_IN_NODE && ext_node[height - 1].node.node_entries[ext_node[height - 1].index].offset != key) {
            ret = RECORD_NOT_FOUND;
            goto ERROR_HANDLER;
        }

        strcpy(value, leaf->node.node_entries[leaf->index].record.value);

ERROR_HANDLER:
        delete[] ext_node;
        return ret;
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
    Container() : storage(nullptr), index_data(nullptr), file(nullptr), height(0), good(false) {}

    Storage<RECORDS_IN_NODE> *storage;
    std::iostream *index_data;
    std::fstream *file;
    uint64_t height;
    bool good;
};

}