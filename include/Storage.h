#pragma once

#include "btree_utils.h"
#include "Node.h"

#include <iosfwd>

namespace btree {

template<uint32_t RECORDS_IN_INDEX_NODE>
struct ExtendedNode {
    uint32_t index;
    Node<RECORDS_IN_INDEX_NODE> node;
};

template<uint32_t RECORDS_IN_INDEX_NODE>
class Storage {
public:
    Storage(std::iostream *_stream, const uint64_t &_height) : stream(_stream), height(_height) {}

    // Returns error code or btree::SUCCESS.
    int find_node(uint64_t key, ExtendedNode<RECORDS_IN_INDEX_NODE> *ext_node) const {
        uint64_t node_offset = 0;
        for (int i = 0; i < height; i++) {
            auto ret = open_node(node_offset, &ext_node[i].node);
            if (ret != btree::SUCCESS) {
                return ret;
            }

            ext_node[i].index = binary_search(key, &ext_node[i].node, i + 1 == height ? true : false);
            if (ext_node[i].index < RECORDS_IN_INDEX_NODE) {
                node_offset = ext_node[i].node.node_entries[ext_node[i].index].offset;
            } else {
                node_offset = ext_node[i].node.offset;
            }
        }

        return btree::SUCCESS;
    }

    // Returns error code or btree::SUCCESS.
    int open_node(uint64_t offset, Node<RECORDS_IN_INDEX_NODE> *node) const {
        try {
            stream->seekg(offset * sizeof(Node<RECORDS_IN_INDEX_NODE>), std::ios_base::beg);
            stream->read(reinterpret_cast<char *>(node), sizeof(Node<RECORDS_IN_INDEX_NODE>));
        }
        catch (...) {
            return btree::SOMETHING_WENT_WRONG;
        }

        return btree::SUCCESS;
    }

    uint64_t get_height() const {
        return height;
    }

    // Returns position where record is or where it should be (first bigger record).
    static uint32_t binary_search(uint64_t key, Node<RECORDS_IN_INDEX_NODE> *node, bool leaf = false) {
        auto get_key = [&](uint32_t i) {
            if (!leaf) {
                return node->node_entries[i].record.key;
            } else {
                return node->node_entries[i].offset;
            }
        };

        if (key < get_key(0)) {
            return 0;
        } else if (key > get_key(node->usage - 1)) {
            return node->usage;
        }

        uint32_t a = 0, b = node->usage, mid = (a + b) / 2;
        while (b - a > 1) {
            if (key < get_key(mid)) {
                b = mid;
            } else if (key > get_key(mid)) {
                a = mid;
            } else {
                b = mid;
                break;
            }

            mid = (a + b) / 2;
        }

        if (key == get_key(a)) {
            return a;
        }

        return b;
    }

private:
    std::iostream *stream;
    const uint64_t &height;
};

}