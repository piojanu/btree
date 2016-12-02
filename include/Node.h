#pragma once

#include "Record.h"

#include <cstdint>
#include <type_traits>

namespace btree {

struct NodeEntry {
    NodeEntry() : offset(-1), record(0, "") {};
    NodeEntry(int64_t _offset, Record _record) : offset(_offset), record(_record) {};

    // Offset of left node in index file.
    int64_t offset;
    // Key, value pair.
    Record record;
};

template<uint8_t RECORDS_IN_INDEX_NODE,
        typename = typename std::enable_if<RECORDS_IN_INDEX_NODE <= 64>::type>
struct Node {
    Node() : offset(-1) {};

    // 0 - free, 1 - occupied
    NodeEntry node_entries[RECORDS_IN_INDEX_NODE];
    int64_t offset;
};

}