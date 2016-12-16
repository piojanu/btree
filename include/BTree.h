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
        if (key == 0) {
            return INVALID_KEY;
        }

        if (height == 0) { // Empty tree
            Node<RECORDS_IN_NODE> root_leaf{};
            fill_leaf_entry(root_leaf.node_entries, key, value);
            root_leaf.usage++;

            auto ret = storage->write_node(0, &root_leaf);
            height = 1;

            return ret;
        } else { // Tree with at least root
            auto node_path = new ExtendedNode<RECORDS_IN_NODE>[height];
            auto leaf = node_path + height - 1;

            auto ret = storage->find_node(key, node_path);
            if (ret != SUCCESS) {
                goto ERROR_HANDLER;
            }

            if (leaf->node.node_entries[leaf->index].offset == key) {
                ret = RECORD_EXISTS;
                goto ERROR_HANDLER;
            }

            ret = insert_entry(leaf, {key, value}, height);

ERROR_HANDLER:
            delete[] node_path;
            return ret;
        }
    }

    // Assign new value for record with given key.
    int update(uint64_t key, const char value[8]) {
        if (key == 0) {
            return INVALID_KEY;
        }

        auto node_path = new ExtendedNode<RECORDS_IN_NODE>[height];
        auto leaf = node_path + height - 1;

        auto ret = storage->find_node(key, node_path);
        if (ret != SUCCESS) {
            goto ERROR_HANDLER;
        }

        if (leaf->node.node_entries[leaf->index].offset == key) {
            strcpy(leaf->node.node_entries[leaf->index].record.value, value);
            ret = storage->write_node(leaf->offset, &leaf->node, true);
        } else {
            ret = RECORD_NOT_FOUND;
            goto ERROR_HANDLER;
        }

ERROR_HANDLER:
        return ret;
    }

    // Remove record from container.
    int remove(uint64_t key) {
        if (key == 0) {
            return INVALID_KEY;
        }

        return NOT_IMPLEMENTED;
    }

    // Get value of given key.
    int get_value(uint64_t key, char value[8]) {
        if (key == 0) {
            return INVALID_KEY;
        }

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
        auto ext_node = new ExtendedNode<RECORDS_IN_NODE>[height];
        auto leaf = ext_node + height - 1;

        auto ret = storage->find_node(1, ext_node);
        if (ret != SUCCESS) {
            goto ERROR_HANDLER;
        }

        auto next_node = &leaf->node;
        do {
            for( auto i = 0ULL; i < next_node->usage; i++ ) {
                out << next_node->node_entries[i].offset << " " << std::string(next_node->node_entries[i].record.value) << " ";
            }

            if( next_node->next_offset != 0 ) {
                storage->open_node(next_node->next_offset, next_node);
            } else {
                break;
            }
        } while (true);

ERROR_HANDLER:
        delete[] ext_node;
        return ret;
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
                node->node_entries[i + step + 1].offset = node->node_entries[i + 1].offset;
            }

            memcpy(&node->node_entries[i + step], &node->node_entries[i], sizeof(NodeEntry));
        }

        if (clear) {
            memset(&node->node_entries[start_index], 0, step * sizeof(NodeEntry));
        }

        return SUCCESS;
    }

    // Fills entry in leaf at specified index.
    inline void
    fill_leaf_entry(NodeEntry *entry, uint64_t key, const char *value) const {
        entry->offset = key;
        strcpy(entry->record.value, value);
    }

    // Fills key in inner node at specified index and offset at one bigger index.
    inline void
    fill_node_entry(NodeEntry *entry, uint64_t key, uint64_t offset) const {
        (entry + 1)->offset = offset;
        entry->record.key = key;
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
            right->offset = parent->node.node_entries[parent->index + 1].offset;
            right->index = parent->index + 1;
            storage->open_node(right->offset, &right->node);
        }
    }

    // Merge two nodes and split equally.
    // 'in_right' flag indicates to witch node new element should be added (left = 0, right = 1).
    // If mid_key is set it indicates inner node.
    // NOTE: If it's inner node, then mid_key from parent will be automatically set to proper value.
    //       If it's leaf, then you have to explicitly set key in parent node.
    inline void
    compensate(Node<RECORDS_IN_NODE> *left, Node<RECORDS_IN_NODE> *right, const NodeEntry new_entry, uint32_t index,
               bool in_right, uint64_t *mid_key = nullptr) const {
        auto sum_usage = left->usage + right->usage + 1;
        auto right_new_usage = static_cast<uint64_t>((sum_usage + 1) / 2);
        auto left_new_usage = sum_usage - right_new_usage;

        // Fill left half, mid and right half of one long node
        auto node_entries = new NodeEntry[sum_usage + 2 * (mid_key != nullptr)];
        auto node_iter = 0ULL;

        // Left half
        if (!in_right) {
            memcpy(node_entries, left->node_entries, index * sizeof(NodeEntry));
            node_iter += index;

            // Add new entry
            auto new_entry_index = node_iter;
            if (mid_key != nullptr) {
                node_entries[new_entry_index].offset = left->node_entries[index].offset;
            } else {
                fill_leaf_entry(node_entries + new_entry_index, new_entry.offset, new_entry.record.value);
            }
            node_iter += 1;

            memcpy(node_entries + node_iter, left->node_entries + index,
                   (left->usage - index + (mid_key != nullptr)) * sizeof(NodeEntry));
            node_iter = left->usage + 1;

            // If it is inner node, now end adding new entry.
            if (mid_key != nullptr) {
                fill_node_entry(node_entries + new_entry_index, new_entry.record.key, new_entry.offset);
            }
        } else {
            memcpy(node_entries, left->node_entries, (left->usage + (mid_key != nullptr)) * sizeof(NodeEntry));
            node_iter = left->usage;
        }

        //Mid
        if (mid_key != nullptr) {
            node_entries[node_iter].record.key = *mid_key;
            node_iter++;
        }

        // Right half
        if (in_right) {
            memcpy(node_entries + node_iter, right->node_entries, index * sizeof(NodeEntry));
            node_iter += index;

            // Add new entry
            auto new_entry_index = node_iter;
            if (mid_key != nullptr) {
                node_entries[new_entry_index].offset = right->node_entries[index].offset;
            } else {
                fill_leaf_entry(node_entries + new_entry_index, new_entry.offset, new_entry.record.value);
            }
            node_iter += 1;

            memcpy(node_entries + node_iter, right->node_entries + index,
                   (right->usage - index + (mid_key != nullptr)) * sizeof(NodeEntry));

            // If it is inner node, now end adding new entry.
            if (mid_key != nullptr) {
                fill_node_entry(node_entries + new_entry_index, new_entry.record.key, new_entry.offset);
            }
        } else {
            memcpy(node_entries + node_iter, right->node_entries,
                   (right->usage + (mid_key != nullptr)) * sizeof(NodeEntry));
        }

        // Set new mid_key and erase it in note_entries
        if (mid_key != nullptr) {
            *mid_key = node_entries[left_new_usage].record.key;
            node_entries[left_new_usage].record.key = 0;
        }

        // Clear nodes...
        memset(left->node_entries, 0, (left->usage + (mid_key != nullptr)) * sizeof(NodeEntry));
        memset(right->node_entries, 0, (right->usage + (mid_key != nullptr)) * sizeof(NodeEntry));

        // ...and distribute each new half to proper node.
        memcpy(left->node_entries, node_entries, (left_new_usage + (mid_key != nullptr)) * sizeof(NodeEntry));
        memcpy(right->node_entries, node_entries + left_new_usage + (mid_key != nullptr),
               (right_new_usage + (mid_key != nullptr)) * sizeof(NodeEntry));

        // Set new usage.
        left->usage = left_new_usage;
        right->usage = right_new_usage;

        delete[] node_entries;
    }

    // Split 'old_node'. Uses 'new_node' to it.
    // 'is_leaf' indicates what type of node is being split.
    // 'mid_entry' is new entry pointing at 'new_node', which should be added to parent.
    inline void
    split(Node<RECORDS_IN_NODE> *old_node, Node<RECORDS_IN_NODE> *new_node, const NodeEntry new_entry, uint32_t index,
          uint64_t &mid_key, bool is_leaf = true) const {
        auto sum_usage = old_node->usage + 1;
        auto old_new_usage = static_cast<uint64_t>((sum_usage + 1) / 2);
        auto new_new_usage =
                sum_usage - old_new_usage;
        old_new_usage -= (!is_leaf); // - 1 if we give mid entry away, because we are inner node

        // Fill long node
        auto node_entries = new NodeEntry[sum_usage + (!is_leaf)];
        auto node_iter = 0ULL;

        // Copy left half of old (new entry divides)
        memcpy(node_entries, old_node->node_entries, index * sizeof(NodeEntry));
        node_iter += index;

        // Add new entry
        auto new_entry_index = node_iter;
        if (!is_leaf) {
            node_entries[new_entry_index].offset = old_node->node_entries[index].offset;
        } else {
            fill_leaf_entry(node_entries + new_entry_index, new_entry.offset, new_entry.record.value);
        }
        node_iter += 1;

        memcpy(node_entries + node_iter, old_node->node_entries + index,
               (old_node->usage - index + (!is_leaf)) * sizeof(NodeEntry));

        // If it is inner node, now end adding new entry.
        if (!is_leaf) {
            fill_node_entry(node_entries + new_entry_index, new_entry.record.key, new_entry.offset);
        }

        // Set mid_key and erase it in note_entries if it is inner node
        if (!is_leaf) {
            mid_key = node_entries[old_new_usage].record.key;
            node_entries[old_new_usage].record.key = 0;
        } else {
            mid_key = node_entries[old_new_usage - 1].offset;
        }

        // Clear nodes...
        memset(old_node->node_entries, 0, (old_node->usage + (!is_leaf)) * sizeof(NodeEntry));

        // ...and distribute each new half to proper node.
        memcpy(old_node->node_entries, node_entries, (old_new_usage + (!is_leaf)) * sizeof(NodeEntry));
        memcpy(new_node->node_entries, node_entries + old_new_usage + (!is_leaf),
               (new_new_usage + (!is_leaf)) * sizeof(NodeEntry));

        // Set new usage.
        old_node->usage = old_new_usage;
        new_node->usage = new_new_usage;

        delete[] node_entries;
    }

    // Inserts entry into specified index.
    // Take care about all all splits and compensations.
    int insert_entry(ExtendedNode<RECORDS_IN_NODE> *node, const NodeEntry new_entry, uint64_t level,
                     bool is_leaf = true) {
        auto ret = 0;

        if (node->node.usage < RECORDS_IN_NODE) { // Node has space for new record.
            ret = shift_to_right(&node->node, node->index, 1, false, !is_leaf);
            if (ret != SUCCESS) {
                goto RETURN;
            }

            if (is_leaf) {
                fill_leaf_entry(node->node.node_entries + node->index, new_entry.offset, new_entry.record.value);
            } else {
                fill_node_entry(node->node.node_entries + node->index, new_entry.record.key, new_entry.offset);
            }
            node->node.usage++;

            ret = storage->write_node(node->offset, &node->node, true);
        } else { // Leaf doesn't have space for new record.
            // !!! If we have only root, split it.
            if (level == 1) {
                ExtendedNode<RECORDS_IN_NODE> ext_node[2] = {};
                uint64_t mid_key = 0;

                // Get and reserve free offsets for nodes.
                ext_node[0].offset = storage->get_free_offset(true);
                ext_node[1].offset = storage->get_free_offset(true);

                // Copy current root to left node
                memcpy(ext_node[0].node.node_entries, node->node.node_entries, sizeof(ext_node[0].node.node_entries));
                ext_node[0].node.usage = node->node.usage;

                // Split
                split(&ext_node[0].node, &ext_node[1].node, new_entry, node->index, mid_key, is_leaf);
                if (is_leaf) {
                    ext_node[1].node.next_offset = ext_node[0].node.next_offset;
                    ext_node[0].node.next_offset = ext_node[1].offset;
                }

                // Create new root
                memset(&node->node, 0, sizeof(Node<RECORDS_IN_NODE>));
                node->node.usage = 1;
                node->node.node_entries[0].offset = ext_node[0].offset;
                node->node.node_entries[0].record.key = mid_key;
                node->node.node_entries[1].offset = ext_node[1].offset;

                // Persist
                storage->write_node(node->offset, &node->node, true);
                storage->write_node(ext_node[0].offset, &ext_node[0].node, true);
                storage->write_node(ext_node[1].offset, &ext_node[1].node, true);

                // Increment height
                height++;

                ret = SUCCESS;
                goto RETURN;
            }

            // !!! Fetch brothers.
            ExtendedNode<RECORDS_IN_NODE> left_brother{}, right_brother{};
            fetch_brothers(node, &left_brother, &right_brother);
            auto parent = node - 1;

            // Check brothers if exist and have space. If so, compensate.
            if (left_brother.node.usage > 0 && left_brother.node.usage < RECORDS_IN_NODE) {
                // Set mid_key if it isn't node
                uint64_t *mid_key = nullptr;
                if (!is_leaf) {
                    mid_key = &parent->node.node_entries[left_brother.index].record.key;
                }

                compensate(&left_brother.node, &node->node, new_entry, node->index, true, mid_key);

                // Set new parent if it is node
                if (is_leaf) {
                    parent->node.node_entries[left_brother.index].record.key =
                            left_brother.node.node_entries[left_brother.node.usage - 1].offset;
                }

                // Persist
                storage->write_node(parent->offset, &parent->node, true);
                storage->write_node(node->offset, &node->node, true);
                storage->write_node(left_brother.offset, &left_brother.node, true);

                ret = SUCCESS;
                goto RETURN;
            } else if (right_brother.node.usage > 0 && right_brother.node.usage < RECORDS_IN_NODE) {
                // Set mid_key if it isn't node
                uint64_t *mid_key = nullptr;
                if (!is_leaf) {
                    mid_key = &parent->node.node_entries[parent->index].record.key;
                }

                compensate(&node->node, &right_brother.node, new_entry, node->index, false, mid_key);

                // Set new parent if it is node
                if (is_leaf) {
                    parent->node.node_entries[parent->index].record.key =
                            node->node.node_entries[node->node.usage - 1].offset;
                }

                // Persist
                storage->write_node(parent->offset, &parent->node, true);
                storage->write_node(node->offset, &node->node, true);
                storage->write_node(right_brother.offset, &right_brother.node, true);

                ret = SUCCESS;
                goto RETURN;
            }

            // !!! No space in brothers. So let's split!
            ExtendedNode<RECORDS_IN_NODE> ext_node{};
            uint64_t mid_key = 0;

            // Get and reserve free offset for node.
            ext_node.offset = storage->get_free_offset(true);
            split(&node->node, &ext_node.node, new_entry, node->index, mid_key, is_leaf);
            if (is_leaf) {
                ext_node.node.next_offset = node->node.next_offset;
                node->node.next_offset = ext_node.offset;
            }

            // Persist leafs.
            storage->write_node(node->offset, &node->node, true);
            storage->write_node(ext_node.offset, &ext_node.node, true);

            // Insert new record into parent.
            NodeEntry parent_new_entry = {ext_node.offset, mid_key};
            ret = insert_entry(parent, parent_new_entry, level - 1, false);
        }

RETURN:
        return ret;
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