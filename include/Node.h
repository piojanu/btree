#pragma once

#include "Record.h"

#include <cstdint>
#include <type_traits>

namespace btree {

struct NodeEntry {
    NodeEntry() = default;
    NodeEntry(uint64_t _offset, uint64_t _key) : offset(_offset) {
        record.key = _key;
    }
    NodeEntry(uint64_t _offset, const char _value[8]) : offset(_offset) {
        strcpy(record.value, _value);
    }

    // Offset of left node in an inner node or key of record in a leaf.
    uint64_t offset;
    // Key in an inner node or value in a leaf.
    Record record;
};

template<uint32_t RECORDS_IN_NODE>
struct Node {
    Node() = default;

    uint64_t usage;
    // Outer right entry is just an offset in inner node or nothing in leaf
    NodeEntry node_entries[RECORDS_IN_NODE + 1];
    // Leaf nodes creates one-way list, from lower to higher
    uint64_t next_offset;
};

}