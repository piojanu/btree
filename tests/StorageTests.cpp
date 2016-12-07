#include "gtest/gtest.h"
#include "Storage.h"

#include <sstream>

struct StorageBasicTest : public ::testing::Test {
    StorageBasicTest() : values{1, 1, 2, 2, 0, 0, 0, 0,
                                2, 1, 1, 2, 2, 0, 0, 0,
                                3, 3, 3, 4, 4, 5, 5, 0}, height(2),
                         stream(""), node(), storage(stream, height) {
        for (uint64_t value : values) {
            stream.write(reinterpret_cast<char *>(&value), sizeof(value));
        }
    }

    std::stringstream stream;
    uint64_t values[24], height;
    static const uint32_t RECORDS_IN_INDEX_NODE = 3;

    btree::Node<RECORDS_IN_INDEX_NODE> node;
    btree::Storage<RECORDS_IN_INDEX_NODE> storage;
};

struct StorageBinarySearchTest : public ::testing::Test {
    StorageBinarySearchTest() {
        node.usage = 5;
        node.node_entries[0].offset = 1;
        node.node_entries[0].record.key = 1;
        node.node_entries[1].offset = 2;
        node.node_entries[1].record.key = 5;
        node.node_entries[2].offset = 3;
        node.node_entries[2].record.key = 7;
        node.node_entries[3].offset = 4;
        node.node_entries[3].record.key = 12;
        node.node_entries[4].offset = 5;
        node.node_entries[4].record.key = 20;
        node.offset = 6;

        leaf_node.usage = 7;
        leaf_node.node_entries[0].offset = 4;
        leaf_node.node_entries[0].record.value[0] = 'g';
        leaf_node.node_entries[1].offset = 15;
        leaf_node.node_entries[1].record.value[0] = 'f';
        leaf_node.node_entries[2].offset = 34;
        leaf_node.node_entries[2].record.value[0] = 'e';
        leaf_node.node_entries[3].offset = 35;
        leaf_node.node_entries[3].record.value[0] = 'd';
        leaf_node.node_entries[4].offset = 36;
        leaf_node.node_entries[4].record.value[0] = 'c';
        leaf_node.node_entries[5].offset = 78;
        leaf_node.node_entries[5].record.value[0] = 'b';
        leaf_node.node_entries[6].offset = 90;
        leaf_node.node_entries[6].record.value[0] = 'a';
        leaf_node.offset = 0;
    }

    static const uint32_t RECORDS_IN_INDEX_NODE = 7;
    btree::Node<RECORDS_IN_INDEX_NODE> node, leaf_node;
};

TEST_F(StorageBasicTest, GIVENstorageWHENchangeAndGetHeightTHENproperHeight) {
    height = 7;
    EXPECT_EQ(height, storage.get_height());
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENfindNodeKey2THENproperlyOpenedNodePath) {
    auto node = new btree::ExtendedNode<RECORDS_IN_INDEX_NODE>[height];
    auto ret = storage.find_node(2, node);
    ASSERT_EQ(ret, btree::SUCCESS);

    EXPECT_EQ(0, node[0].index);
    EXPECT_EQ(values[0], node[0].node.usage);
    EXPECT_EQ(values[1], node[0].node.node_entries[0].offset);
    EXPECT_EQ(values[2], node[0].node.node_entries[0].record.key);
    EXPECT_EQ(values[3], node[0].node.node_entries[1].offset);
    EXPECT_EQ(values[4], node[0].node.node_entries[1].record.key);
    EXPECT_EQ(values[5], node[0].node.node_entries[2].offset);
    EXPECT_EQ(values[6], node[0].node.node_entries[2].record.key);
    EXPECT_EQ(values[7], node[0].node.offset);

    EXPECT_EQ(1, node[1].index);
    EXPECT_EQ(values[8], node[1].node.usage);
    EXPECT_EQ(values[8 + 1], node[1].node.node_entries[0].offset);
    EXPECT_EQ(values[8 + 2], node[1].node.node_entries[0].record.key);
    EXPECT_EQ(values[8 + 3], node[1].node.node_entries[1].offset);
    EXPECT_EQ(values[8 + 4], node[1].node.node_entries[1].record.key);
    EXPECT_EQ(values[8 + 5], node[1].node.node_entries[2].offset);
    EXPECT_EQ(values[8 + 6], node[1].node.node_entries[2].record.key);
    EXPECT_EQ(values[8 + 7], node[1].node.offset);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENfindNodeKey5THENproperlyOpenedNodePath) {
    auto node = new btree::ExtendedNode<RECORDS_IN_INDEX_NODE>[height];
    auto ret = storage.find_node(5, node);
    ASSERT_EQ(ret, btree::SUCCESS);

    EXPECT_EQ(1, node[0].index);
    EXPECT_EQ(values[0], node[0].node.usage);
    EXPECT_EQ(values[1], node[0].node.node_entries[0].offset);
    EXPECT_EQ(values[2], node[0].node.node_entries[0].record.key);
    EXPECT_EQ(values[3], node[0].node.node_entries[1].offset);
    EXPECT_EQ(values[4], node[0].node.node_entries[1].record.key);
    EXPECT_EQ(values[5], node[0].node.node_entries[2].offset);
    EXPECT_EQ(values[6], node[0].node.node_entries[2].record.key);
    EXPECT_EQ(values[7], node[0].node.offset);

    EXPECT_EQ(2, node[1].index);
    EXPECT_EQ(values[16], node[1].node.usage);
    EXPECT_EQ(values[16 + 1], node[1].node.node_entries[0].offset);
    EXPECT_EQ(values[16 + 2], node[1].node.node_entries[0].record.key);
    EXPECT_EQ(values[16 + 3], node[1].node.node_entries[1].offset);
    EXPECT_EQ(values[16 + 4], node[1].node.node_entries[1].record.key);
    EXPECT_EQ(values[16 + 5], node[1].node.node_entries[2].offset);
    EXPECT_EQ(values[16 + 6], node[1].node.node_entries[2].record.key);
    EXPECT_EQ(values[16 + 7], node[1].node.offset);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENfindNodeKey6THENproperlyOpenedNodePath) {
    auto node = new btree::ExtendedNode<RECORDS_IN_INDEX_NODE>[height];
    auto ret = storage.find_node(6, node);
    ASSERT_EQ(ret, btree::SUCCESS);

    EXPECT_EQ(1, node[0].index);
    EXPECT_EQ(values[0], node[0].node.usage);
    EXPECT_EQ(values[1], node[0].node.node_entries[0].offset);
    EXPECT_EQ(values[2], node[0].node.node_entries[0].record.key);
    EXPECT_EQ(values[3], node[0].node.node_entries[1].offset);
    EXPECT_EQ(values[4], node[0].node.node_entries[1].record.key);
    EXPECT_EQ(values[5], node[0].node.node_entries[2].offset);
    EXPECT_EQ(values[6], node[0].node.node_entries[2].record.key);
    EXPECT_EQ(values[7], node[0].node.offset);

    EXPECT_EQ(3, node[1].index);
    EXPECT_EQ(values[16], node[1].node.usage);
    EXPECT_EQ(values[16 + 1], node[1].node.node_entries[0].offset);
    EXPECT_EQ(values[16 + 2], node[1].node.node_entries[0].record.key);
    EXPECT_EQ(values[16 + 3], node[1].node.node_entries[1].offset);
    EXPECT_EQ(values[16 + 4], node[1].node.node_entries[1].record.key);
    EXPECT_EQ(values[16 + 5], node[1].node.node_entries[2].offset);
    EXPECT_EQ(values[16 + 6], node[1].node.node_entries[2].record.key);
    EXPECT_EQ(values[16 + 7], node[1].node.offset);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENopenNodeTHENproperlyOpenedNode) {
    auto ret = storage.open_node(1, &node);
    ASSERT_EQ(ret, btree::SUCCESS);

    EXPECT_EQ(values[8], node.usage);
    EXPECT_EQ(values[8 + 1], node.node_entries[0].offset);
    EXPECT_EQ(values[8 + 2], node.node_entries[0].record.key);
    EXPECT_EQ(values[8 + 3], node.node_entries[1].offset);
    EXPECT_EQ(values[8 + 4], node.node_entries[1].record.key);
    EXPECT_EQ(values[8 + 5], node.node_entries[2].offset);
    EXPECT_EQ(values[8 + 6], node.node_entries[2].record.key);
    EXPECT_EQ(values[8 + 7], node.offset);
}

TEST_F(StorageBinarySearchTest, GIVENnodeWithEntriesWHENbinarySearchProperKeyTHENpointsProperEntry) {
    const uint32_t index = 1;
    auto ret = btree::Storage<RECORDS_IN_INDEX_NODE>::binary_search(
            node.node_entries[index].record.key, &node);
    EXPECT_EQ(index, ret);
}

TEST_F(StorageBinarySearchTest, GIVENnodeWithEntriesWHENbinarySearchMissingKeyTHENpointsFirstBiggerEntry) {
    const uint32_t index = 1;
    auto ret = btree::Storage<RECORDS_IN_INDEX_NODE>::binary_search(
            node.node_entries[index].record.key + 1, &node);
    EXPECT_EQ(index + 1, ret);
}

TEST_F(StorageBinarySearchTest, GIVENnodeWithEntriesWHENbinarySearchFirstTHENpointsFirstBiggerEntry) {
    const uint32_t index = 0;
    auto ret = btree::Storage<RECORDS_IN_INDEX_NODE>::binary_search(
            node.node_entries[index].record.key, &node);
    EXPECT_EQ(index, ret);
}

TEST_F(StorageBinarySearchTest, GIVENnodeWithEntriesWHENbinarySearchSmallerThenFirstTHENpointsFirstBiggerEntry) {
    const uint32_t index = 0;
    auto ret = btree::Storage<RECORDS_IN_INDEX_NODE>::binary_search(
            node.node_entries[index].record.key - 1, &node);
    EXPECT_EQ(index, ret);
}

TEST_F(StorageBinarySearchTest, GIVENnodeWithEntriesWHENbinarySearchLastTHENpointsFirstBiggerEntry) {
    const uint32_t index = node.usage - 1;
    auto ret = btree::Storage<RECORDS_IN_INDEX_NODE>::binary_search(
            node.node_entries[index].record.key, &node);
    EXPECT_EQ(index, ret);
}

TEST_F(StorageBinarySearchTest, GIVENnodeWithEntriesWHENbinarySearchBiggerThenLastTHENpointsFirstBiggerEntry) {
    const uint32_t index = node.usage - 1;
    auto ret = btree::Storage<RECORDS_IN_INDEX_NODE>::binary_search(
            node.node_entries[index].record.key + 1, &node);
    EXPECT_EQ(index + 1, ret);
}

TEST_F(StorageBinarySearchTest, GIVENleafWithEntriesWHENbinarySearchProperKeyTHENpointsProperEntry) {
    const uint32_t index = 1;
    auto ret = btree::Storage<RECORDS_IN_INDEX_NODE>::binary_search(
            leaf_node.node_entries[index].offset, &leaf_node, true);
    EXPECT_EQ(index, ret);
}

TEST_F(StorageBinarySearchTest, GIVENleafWithEntriesWHENbinarySearchMissingKeyTHENpointsFirstBiggerEntry) {
    const uint32_t index = 1;
    auto ret = btree::Storage<RECORDS_IN_INDEX_NODE>::binary_search(
            leaf_node.node_entries[index].offset + 1, &leaf_node, true);
    EXPECT_EQ(index + 1, ret);
}