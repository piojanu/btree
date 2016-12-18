#pragma once

#include "btree_utils.h"
#include "Node.h"

#include <iosfwd>
#include <unordered_set>

namespace btree {

template<uint32_t RECORDS_IN_NODE>
struct ExtendedNode {
    uint64_t offset;
    uint32_t index;
    Node<RECORDS_IN_NODE> node;
};

template<uint32_t RECORDS_IN_NODE>
class Storage {
public:
    Storage(std::iostream *_stream, const uint64_t &_height, const uint64_t &nodes_count = 0) : stream(_stream),
                                                                                                height(_height),
                                                                                                free_offsets(),
                                                                                                end_offset(
                                                                                                        nodes_count) {}

    // Returns error code or btree::SUCCESS.
    int find_node(uint64_t key, ExtendedNode<RECORDS_IN_NODE> *ext_node) const {
        if (height == 0) {
            return btree::EMPTY_STORAGE;
        }

        uint64_t offset = 0;
        for (int i = 0; i < height; i++) {
            auto ret = open_node(offset, &ext_node[i].node);
            if (ret != btree::SUCCESS) {
                return ret;
            }

            ext_node[i].offset = offset;
            ext_node[i].index = binary_search(key, &ext_node[i].node, i + 1 == height);
            offset = ext_node[i].node.node_entries[ext_node[i].index].offset;
        }

        return btree::SUCCESS;
    }

    // Returns error code or btree::SUCCESS.
    int open_node(uint64_t offset, Node<RECORDS_IN_NODE> *node) const {
        if (offset >= end_offset || free_offsets.find(offset) != free_offsets.end()) {
            return INVALID_OFFSET;
        }

        try {
            stream->seekg(offset * sizeof(Node<RECORDS_IN_NODE>), std::ios_base::beg);
            stream->read(reinterpret_cast<char *>(node), sizeof(Node<RECORDS_IN_NODE>));
            g_iinfo.reads++;
        }
        catch (...) {
            return btree::SOMETHING_WENT_WRONG;
        }

        return btree::SUCCESS;
    }

    // Writes node object into specified offset.
    // Write can't be after end offset.
    // Write can't override existing node,
    int write_node(uint64_t offset, Node<RECORDS_IN_NODE> *node, bool force = false) {
        if (offset == end_offset) {
            end_offset++;
        } else if (offset < end_offset && (force || free_offsets.find(offset) != free_offsets.end())) {
            free_offsets.erase(offset);
        } else {
            return INVALID_OFFSET;
        }

        try {
            stream->seekp(offset * sizeof(Node<RECORDS_IN_NODE>), std::ios_base::beg);
            stream->write(reinterpret_cast<char *>(node), sizeof(Node<RECORDS_IN_NODE>));
            g_iinfo.writes++;
        }
        catch (...) {
            return btree::SOMETHING_WENT_WRONG;
        }

        return btree::SUCCESS;
    }

    // Deletes node at specified offset.
    int delete_node(uint64_t offset) {
        if (offset >= end_offset || free_offsets.find(offset) != free_offsets.end()) {
            return INVALID_OFFSET;
        }

        // TODO: Horrible! This monster is here because of close dead line and bad tests assumption.
        Node<RECORDS_IN_NODE> empty{};
        write_node(offset, &empty, true);

        free_offsets.insert(offset);
        return SUCCESS;
    }

    // Returns position where record is or where it should be (first bigger record).
    static uint32_t binary_search(uint64_t key, Node<RECORDS_IN_NODE> *node, bool leaf = false) {
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

    uint64_t get_height() const {
        return height;
    }

    uint64_t get_free_offset(bool reserve = false, bool force_end_offset = false) {
        uint64_t offset = 0;

        if (free_offsets.empty() || force_end_offset) {
            offset = end_offset;
        } else {
            offset = *free_offsets.begin();
        }

        if (reserve) {
            reserve_offset(offset);
        }
        return offset;
    }

    // NODE: After reserving offset you have to set 'force' param in write_node method if you want to write to this offset.
    int reserve_offset(uint64_t offset) {
        if (offset == end_offset) {
            end_offset++;
            return SUCCESS;
        } else if (free_offsets.find(offset) != free_offsets.end()) {
            free_offsets.erase(offset);
            return SUCCESS;
        } else {
            return INVALID_OFFSET;
        }
    }

private:
    std::iostream *stream;
    const uint64_t &height;

    std::unordered_set<uint64_t> free_offsets;
    uint64_t end_offset;
};

}