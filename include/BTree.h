#pragma once

#include "Storage.h"
#include "Record.h"
#include "Node.h"

#include <cstdint>
#include <fstream>

namespace btree {

template<const uint32_t RECORDS_IN_NODE,
        typename = typename std::enable_if<(RECORDS_IN_NODE > 1)>::type>
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
        good = true;
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
            Node<RECORDS_IN_NODE> root_leaf{};
            fill_leaf_entry(&root_leaf, 0, key, value);

            auto ret = storage->write_node(0, &root_leaf);
            height = 1;

            return ret;
        } else { // Tree with at least root
            auto node_path = new ExtendedNode<RECORDS_IN_NODE>[height];
            auto leaf = node_path + height - 1;

            auto ret = storage->find_node(key, node_path);
            if (ret != SUCCESS) {
                goto RETURN;
            }

            if (leaf->node.node_entries[leaf->index].offset == key) {
                ret = RECORD_EXISTS;
                goto RETURN;
            }

            if (leaf->node.usage < RECORDS_IN_NODE) { // Leaf has space for new record.
                ret = shift_to_right(&leaf->node, leaf->index);
                if (ret != SUCCESS) {
                    goto RETURN;
                }

                fill_leaf_entry(&leaf->node, leaf->index, key, value);

                ret = storage->write_node(leaf->offset, &leaf->node, true);
            } else { // Leaf doesn't have space for new record.
                // !!! If we have only root, split it.
                if (height == 1) {
                    ExtendedNode<RECORDS_IN_NODE> ext_node[2] = {};
                    auto sum_usage = leaf->node.usage + 1;
                    auto right_usage = static_cast<uint64_t>(sum_usage / 2);
                    auto left_usage = sum_usage - right_usage;

                    // Get and reserve free offsets for nodes.
                    ext_node[0].offset = storage->get_free_offset(true);
                    ext_node[1].offset = storage->get_free_offset(true);

                    // Copy records to left node
                    auto leaf_iter = 0u;
                    auto new_entry_copied = false;
                    for (int i = 0; i < left_usage; i++) {
                        // Check if we are copying leaf new entry.
                        if (leaf_iter == leaf->index && !new_entry_copied) {
                            fill_leaf_entry(&ext_node[0].node, i, key, value);
                            new_entry_copied = true;
                        } else {
                            memcpy(&ext_node[0].node.node_entries[i], &leaf->node.node_entries[leaf_iter],
                                   sizeof(NodeEntry));
                            leaf_iter++;
                        }
                    }
                    ext_node[0].node.usage = left_usage;

                    // Copy records to right node
                    for (int i = 0; i < right_usage; i++) {
                        // Check if we are copying leaf new entry.
                        if (leaf_iter == leaf->index && !new_entry_copied) {
                            fill_leaf_entry(&ext_node[1].node, i, key, value);
                            new_entry_copied = true;
                        } else {
                            memcpy(&ext_node[1].node.node_entries[i], &leaf->node.node_entries[leaf_iter],
                                   sizeof(NodeEntry));
                            leaf_iter++;
                        }
                    }
                    ext_node[1].node.usage = right_usage;

                    // Create new root
                    memset(&leaf->node, 0, sizeof(Node<RECORDS_IN_NODE>));
                    leaf->node.usage = 1;
                    leaf->node.node_entries[0].offset = ext_node[0].offset;
                    leaf->node.node_entries[0].record.key = ext_node[0].node.node_entries[left_usage - 1].offset;
                    leaf->node.node_entries[1].offset = ext_node[1].offset;

                    // Persist
                    storage->write_node(leaf->offset, &leaf->node, true);
                    storage->write_node(ext_node[0].offset, &ext_node[0].node, true);
                    storage->write_node(ext_node[1].offset, &ext_node[1].node, true);

                    ret = SUCCESS;
                    goto RETURN;
                }

                // !!! Fetch brothers.
                ExtendedNode<RECORDS_IN_NODE> left_brother{}, right_brother{};
                // TODO: This could be optimised. There is no need to fetch always both brothers.
                fetch_brothers(leaf, &left_brother, &right_brother);

                // Check brothers if exist and have space. If so, compensate.
                if (left_brother.node.usage > 0 && left_brother.node.usage < RECORDS_IN_NODE) {
                    auto sum_usage = leaf->node.usage + left_brother.node.usage + 1;
                    auto new_usage = static_cast<uint64_t>(sum_usage / 2);

                    auto leaf_iter = 0u;
                    auto new_entry_copied = false;
                    for (auto i = left_brother.node.usage; i < new_usage; i++) {
                        // Check if we are copying leaf new entry.
                        if (leaf_iter == leaf->index && !new_entry_copied) {
                            fill_leaf_entry(&left_brother.node, i, key, value);
                            new_entry_copied = true;
                        } else {
                            memcpy(&left_brother.node.node_entries[i], &leaf->node.node_entries[leaf_iter],
                                   sizeof(NodeEntry));
                            leaf_iter++;
                        }
                    }

                    if (!new_entry_copied) {
                        shift_to_left(&leaf->node, leaf->index);
                        fill_leaf_entry(&leaf->node, leaf->index - 1, key, value);
                        leaf_iter--;
                    }

                    // Shift leaf to left edge
                    if (leaf_iter > 0) {
                        shift_to_left(&leaf->node, leaf->node.usage, leaf_iter, true);
                    }

                    // Set new nodes usage
                    leaf->node.usage = sum_usage - new_usage;
                    left_brother.node.usage = new_usage;

                    // Set new parent
                    auto parent = leaf - 1;
                    parent->node.node_entries[left_brother.index].record.key =
                            left_brother.node.node_entries[left_brother.node.usage - 1].offset;

                    // Persist
                    storage->write_node(parent->offset, &parent->node, true);
                    storage->write_node(leaf->offset, &leaf->node, true);
                    storage->write_node(left_brother.offset, &left_brother.node, true);

                    ret = SUCCESS;
                    goto RETURN;
                } else if (right_brother.node.usage > 0 && right_brother.node.usage < RECORDS_IN_NODE) {
                    auto sum_usage = leaf->node.usage + right_brother.node.usage + 1;
                    auto new_usage = static_cast<uint64_t>(sum_usage / 2);
                    auto delta_usage = new_usage - right_brother.node.usage;

                    // Shift brother to right, so there is space for moving entries from leaf.
                    shift_to_right(&right_brother.node, 0, delta_usage);

                    auto leaf_iter = leaf->node.usage - 1;
                    auto new_entry_copied = false;
                    for (auto i = 0; i < delta_usage; i++) {
                        // Check if we are copying leaf new entry.
                        if (leaf_iter + 1 == leaf->index && !new_entry_copied) {
                            fill_leaf_entry(&right_brother.node, i, key, value);
                            new_entry_copied = true;
                        } else {
                            memcpy(&right_brother.node.node_entries[i], &leaf->node.node_entries[leaf_iter],
                                   sizeof(NodeEntry));
                            // Clear
                            memset(&leaf->node.node_entries[leaf_iter], 0, sizeof(NodeEntry));
                            leaf_iter--;
                        }
                    }

                    if (!new_entry_copied) {
                        if (leaf->index < RECORDS_IN_NODE - 1) {
                            // Make space for shift, it is there now for sure
                            leaf->node.usage--;
                            shift_to_right(&leaf->node, leaf->index);
                        }
                        fill_leaf_entry(&leaf->node, leaf->index, key, value);
                    }

                    // Set new nodes usage
                    leaf->node.usage = sum_usage - new_usage;
                    right_brother.node.usage = new_usage;

                    // Set new parent
                    auto parent = leaf - 1;
                    parent->node.node_entries[parent->index].record.key =
                            leaf->node.node_entries[leaf->node.usage - 1].offset;

                    // Persist
                    storage->write_node(parent->offset, &parent->node, true);
                    storage->write_node(leaf->offset, &leaf->node, true);
                    storage->write_node(right_brother.offset, &right_brother.node, true);

                    ret = SUCCESS;
                    goto RETURN;
                }

                // !!! No space in brothers. So let's split!
                {
                    ExtendedNode<RECORDS_IN_NODE> ext_node{};
                    auto sum_usage = leaf->node.usage + 1;
                    auto right_usage = static_cast<uint64_t>(sum_usage / 2);
                    auto left_usage = sum_usage - right_usage;

                    // Get and reserve free offset for node.
                    ext_node.offset = storage->get_free_offset(true);

                    // Copy records to new node.
                    auto leaf_iter = leaf->node.usage - 1;
                    auto new_entry_copied = false;
                    for (auto i = 0; i < right_usage; i++) {
                        // Check if we are copying leaf new entry.
                        if (leaf_iter + 1 == leaf->index && !new_entry_copied) {
                            fill_leaf_entry(&ext_node.node, i, key, value);
                            new_entry_copied = true;
                        } else {
                            memcpy(&ext_node.node.node_entries[i], &leaf->node.node_entries[leaf_iter],
                                   sizeof(NodeEntry));
                            // Clear
                            memset(&leaf->node.node_entries[leaf_iter], 0, sizeof(NodeEntry));
                            leaf_iter--;
                        }
                    }

                    if (!new_entry_copied) {
                        if (leaf->index < RECORDS_IN_NODE - 1) {
                            // Make space for shift, it is there now for sure
                            leaf->node.usage--;
                            shift_to_right(&leaf->node, leaf->index);
                        }
                        fill_leaf_entry(&leaf->node, leaf->index, key, value);
                    }

                    // Set new nodes usage.
                    leaf->node.usage = left_usage;
                    ext_node.node.usage = right_usage;

                    // Persist leafs.
                    storage->write_node(leaf->offset, &leaf->node, true);
                    storage->write_node(ext_node.offset, &ext_node.node, true);

                    // Insert new record into parent.
                    auto parent = leaf - 1;
                    auto parent_new_key = leaf->node.node_entries[left_usage - 1].offset;
                    auto parent_new_offset = ext_node.offset;
                    if (parent->node.usage < RECORDS_IN_NODE) { // Parent has space.
                        if (parent->index != parent->node.usage) {
                            shift_to_right(&parent->node, parent->index, 1, false, true);
                        }

                        fill_node_entry(&parent->node, parent->index, parent_new_key, parent_new_offset);
                        storage->write_node(parent->offset, &parent->node, true);

                        ret = SUCCESS;
                        goto RETURN;
                    } else { // Parent doesn't have space.


                        ret = NOT_IMPLEMENTED;
                        goto RETURN;
                    }
                }
            }

RETURN:
            delete[] node_path;
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
        if (ext_node[height - 1].index < RECORDS_IN_NODE &&
            ext_node[height - 1].node.node_entries[ext_node[height - 1].index].offset != key) {
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

protected:
    // Makes place for new record. Shift higher values to right (start_index is included).
    // If param 'clear' is true it will clear shifted entries.
    // NOTE: There has to be space for this shift!
    inline int
    shift_to_right(Node<RECORDS_IN_NODE> *node, uint32_t start_index, uint32_t step = 1, bool clear = false,
                   bool inner = false) const {
        if (node->usage + step > RECORDS_IN_NODE) {
            return NOT_ENOUGH_SPACE;
        }

        for (int64_t i = node->usage - step; i >= start_index; i--) {
            // If copy to last record
            if (inner && i == node->usage - step) {
                if (i + step + 1 != RECORDS_IN_NODE) {
                    node->node_entries[i + step + 1].offset = node->node_entries[i + 1].offset;
                } else {
                    node->offset = node->node_entries[i + 1].offset;
                }
            }

            memcpy(&node->node_entries[i + step], &node->node_entries[i], sizeof(NodeEntry));
        }

        if (clear) {
            memset(&node->node_entries[start_index], 0, step * sizeof(NodeEntry));
        }

        return SUCCESS;
    }

    // Makes place for new record. Shift lower values to left (end_index is excluded).
    // If param 'clear' is true it will clear shifted entries.
    // NOTE: There has to be space for this shift!
    inline int
    shift_to_left(Node<RECORDS_IN_NODE> *leaf, uint32_t end_index, uint32_t step = 1, bool clear = false) const {
        for (int64_t i = step; i < end_index; i++) {
            memcpy(&leaf->node_entries[i - step], &leaf->node_entries[i], sizeof(NodeEntry));
        }

        if (clear) {
            memset(&leaf->node_entries[end_index - step], 0, step * sizeof(NodeEntry));
        }

        return SUCCESS;
    }

    // Fills entry in leaf at specified index and increments usage.
    inline void
    fill_leaf_entry(Node<RECORDS_IN_NODE> *leaf, uint32_t entry_index, uint64_t key, const char *value) const {
        leaf->usage++;
        leaf->node_entries[entry_index].offset = key;
        strcpy(leaf->node_entries[entry_index].record.value, value);
    }

    // Fills key in inner node at specified index and offset at one bigger index and increments usage.
    inline void
    fill_node_entry(Node<RECORDS_IN_NODE> *node, uint32_t entry_index, uint64_t key, uint64_t offset) const {
        node->usage++;
        node->node_entries[entry_index].record.key = key;
        if (entry_index < RECORDS_IN_NODE - 1) {
            node->node_entries[entry_index + 1].offset = offset;
        } else {
            node->offset = offset;
        }

    }

    // Fetches brothers if possible. If not, leave Node param untouched.
    // Index member mean position in parent node, differently then in case when it is filled by find_node method.
    // NOTE: Parent has to exist!
    inline void fetch_brothers(ExtendedNode<RECORDS_IN_NODE> *ext_node, ExtendedNode<RECORDS_IN_NODE> *left,
                               ExtendedNode<RECORDS_IN_NODE> *right) const {
        auto parent = ext_node - 1;

        if (parent->index > 0) {
            left->offset = parent->node.node_entries[parent->index - 1].offset;
            left->index = parent->index - 1;
            storage->open_node(left->offset, &left->node);
        }

        if (parent->index < parent->node.usage - 1) {
            right->offset = parent->node.node_entries[parent->index + 1].offset;
            right->index = parent->index + 1;
            storage->open_node(parent->node.node_entries[parent->index + 1].offset, &right->node);
        } else if (parent->index == parent->node.usage - 1) {
            if (parent->index < RECORDS_IN_NODE - 1) {
                right->offset = parent->node.node_entries[parent->index + 1].offset;
            } else {
                right->offset = parent->node.offset;
            }
            right->index = parent->index + 1;
            storage->open_node(right->offset, &right->node);
        }
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