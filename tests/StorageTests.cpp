#include "gtest/gtest.h"
#include "Storage.h"

#include <sstream>

struct StorageBasicTest : public ::testing::Test {
    StorageBasicTest() : values{1, 1, 2, 2, 0, 0, 0, 0, 0,
                                2, 1, 1, 2, 2, 0, 0, 0, 0,
                                3, 3, 3, 4, 4, 5, 5, 0, 0}, height(2),
                         stream(""), node(), storage(&stream, height, nodes_count) {
        for (uint64_t value : values) {
            stream.write(reinterpret_cast<char *>(&value), sizeof(value));
        }

        node.usage = 7;
    }

    std::stringstream stream;
    uint64_t values[27], nodes_count = sizeof(values) / sizeof(btree::Node<RECORDS_IN_INDEX_NODE>), height;
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
        node.node_entries[5].offset = 6;
        node.node_entries[5].record.key = 0;

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
        leaf_node.node_entries[7].offset = 5;
        leaf_node.node_entries[7].record.key = 0;
    }

    static const uint32_t RECORDS_IN_INDEX_NODE = 7;
    btree::Node<RECORDS_IN_INDEX_NODE> node, leaf_node;
};

TEST_F(StorageBasicTest, GIVENstorageWHENchangeAndGetHeightTHENproperHeight) {
    height = 7;
    EXPECT_EQ(height, storage.get_height());
}

TEST_F(StorageBasicTest, GIVENemptyStorageWHENfindNodeKey2THENreturnErrorCode) {
    height = 0;
    auto node = new btree::ExtendedNode<RECORDS_IN_INDEX_NODE>[height];
    auto ret = storage.find_node(2, node);
    EXPECT_EQ(ret, btree::EMPTY_STORAGE);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENfindNodeKey2THENproperlyOpenedNodePath) {
    auto node = new btree::ExtendedNode<RECORDS_IN_INDEX_NODE>[height];
    auto ret = storage.find_node(2, node);
    ASSERT_EQ(ret, btree::SUCCESS);

    EXPECT_EQ(0, node[0].offset);
    EXPECT_EQ(0, node[0].index);
    EXPECT_EQ(values[0], node[0].node.usage);
    EXPECT_EQ(values[1], node[0].node.node_entries[0].offset);
    EXPECT_EQ(values[2], node[0].node.node_entries[0].record.key);
    EXPECT_EQ(values[3], node[0].node.node_entries[1].offset);
    EXPECT_EQ(values[4], node[0].node.node_entries[1].record.key);
    EXPECT_EQ(values[5], node[0].node.node_entries[2].offset);
    EXPECT_EQ(values[6], node[0].node.node_entries[2].record.key);
    EXPECT_EQ(values[7], node[0].node.node_entries[3].offset);
    EXPECT_EQ(values[8], node[0].node.node_entries[3].record.key);

    EXPECT_EQ(1, node[1].offset);
    EXPECT_EQ(1, node[1].index);
    EXPECT_EQ(values[9], node[1].node.usage);
    EXPECT_EQ(values[9 + 1], node[1].node.node_entries[0].offset);
    EXPECT_EQ(values[9 + 2], node[1].node.node_entries[0].record.key);
    EXPECT_EQ(values[9 + 3], node[1].node.node_entries[1].offset);
    EXPECT_EQ(values[9 + 4], node[1].node.node_entries[1].record.key);
    EXPECT_EQ(values[9 + 5], node[1].node.node_entries[2].offset);
    EXPECT_EQ(values[9 + 6], node[1].node.node_entries[2].record.key);
    EXPECT_EQ(values[9 + 7], node[1].node.node_entries[3].offset);
    EXPECT_EQ(values[9 + 8], node[1].node.node_entries[3].record.key);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENfindNodeKey5THENproperlyOpenedNodePath) {
    auto node = new btree::ExtendedNode<RECORDS_IN_INDEX_NODE>[height];
    auto ret = storage.find_node(5, node);
    ASSERT_EQ(ret, btree::SUCCESS);

    EXPECT_EQ(0, node[0].offset);
    EXPECT_EQ(1, node[0].index);
    EXPECT_EQ(values[0], node[0].node.usage);
    EXPECT_EQ(values[1], node[0].node.node_entries[0].offset);
    EXPECT_EQ(values[2], node[0].node.node_entries[0].record.key);
    EXPECT_EQ(values[3], node[0].node.node_entries[1].offset);
    EXPECT_EQ(values[4], node[0].node.node_entries[1].record.key);
    EXPECT_EQ(values[5], node[0].node.node_entries[2].offset);
    EXPECT_EQ(values[6], node[0].node.node_entries[2].record.key);
    EXPECT_EQ(values[7], node[0].node.node_entries[3].offset);
    EXPECT_EQ(values[8], node[0].node.node_entries[3].record.key);

    EXPECT_EQ(2, node[1].offset);
    EXPECT_EQ(2, node[1].index);
    EXPECT_EQ(values[18], node[1].node.usage);
    EXPECT_EQ(values[18 + 1], node[1].node.node_entries[0].offset);
    EXPECT_EQ(values[18 + 2], node[1].node.node_entries[0].record.key);
    EXPECT_EQ(values[18 + 3], node[1].node.node_entries[1].offset);
    EXPECT_EQ(values[18 + 4], node[1].node.node_entries[1].record.key);
    EXPECT_EQ(values[18 + 5], node[1].node.node_entries[2].offset);
    EXPECT_EQ(values[18 + 6], node[1].node.node_entries[2].record.key);
    EXPECT_EQ(values[18 + 7], node[1].node.node_entries[3].offset);
    EXPECT_EQ(values[18 + 8], node[1].node.node_entries[3].record.key);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENfindNodeKey6THENproperlyOpenedNodePath) {
    auto node = new btree::ExtendedNode<RECORDS_IN_INDEX_NODE>[height];
    auto ret = storage.find_node(6, node);
    ASSERT_EQ(ret, btree::SUCCESS);

    EXPECT_EQ(0, node[0].offset);
    EXPECT_EQ(1, node[0].index);
    EXPECT_EQ(values[0], node[0].node.usage);
    EXPECT_EQ(values[1], node[0].node.node_entries[0].offset);
    EXPECT_EQ(values[2], node[0].node.node_entries[0].record.key);
    EXPECT_EQ(values[3], node[0].node.node_entries[1].offset);
    EXPECT_EQ(values[4], node[0].node.node_entries[1].record.key);
    EXPECT_EQ(values[5], node[0].node.node_entries[2].offset);
    EXPECT_EQ(values[6], node[0].node.node_entries[2].record.key);
    EXPECT_EQ(values[7], node[0].node.node_entries[3].offset);
    EXPECT_EQ(values[8], node[0].node.node_entries[3].record.key);

    EXPECT_EQ(2, node[1].offset);
    EXPECT_EQ(3, node[1].index);
    EXPECT_EQ(values[18], node[1].node.usage);
    EXPECT_EQ(values[18 + 1], node[1].node.node_entries[0].offset);
    EXPECT_EQ(values[18 + 2], node[1].node.node_entries[0].record.key);
    EXPECT_EQ(values[18 + 3], node[1].node.node_entries[1].offset);
    EXPECT_EQ(values[18 + 4], node[1].node.node_entries[1].record.key);
    EXPECT_EQ(values[18 + 5], node[1].node.node_entries[2].offset);
    EXPECT_EQ(values[18 + 6], node[1].node.node_entries[2].record.key);
    EXPECT_EQ(values[18 + 7], node[1].node.node_entries[3].offset);
    EXPECT_EQ(values[18 + 8], node[1].node.node_entries[3].record.key);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENopenNotExistingNodeTHENreturnsErrorCode) {
    auto ret = storage.open_node(nodes_count + 1, &node);
    EXPECT_EQ(ret, btree::INVALID_OFFSET);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENopenNodeTHENproperlyOpenedNode) {
    btree::g_iinfo.reset();
    auto ret = storage.open_node(1, &node);
    ASSERT_EQ(ret, btree::SUCCESS);

    EXPECT_EQ(values[9], node.usage);
    EXPECT_EQ(values[9 + 1], node.node_entries[0].offset);
    EXPECT_EQ(values[9 + 2], node.node_entries[0].record.key);
    EXPECT_EQ(values[9 + 3], node.node_entries[1].offset);
    EXPECT_EQ(values[9 + 4], node.node_entries[1].record.key);
    EXPECT_EQ(values[9 + 5], node.node_entries[2].offset);
    EXPECT_EQ(values[9 + 6], node.node_entries[2].record.key);
    EXPECT_EQ(values[9 + 7], node.node_entries[3].offset);
    EXPECT_EQ(values[9 + 8], node.node_entries[3].record.key);

    EXPECT_EQ(btree::g_iinfo.reads, 1);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENwriteToExistingNodeTHENreturnsErrorCode) {
    auto ret = storage.write_node(1, &node);
    EXPECT_EQ(ret, btree::INVALID_OFFSET);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENwriteToExistingNodeWithForceTHENreturnsSuccessAndProperlyWrites) {
    btree::g_iinfo.reset();
    const uint64_t offset = 1;
    auto ret = storage.write_node(offset, &node, true);
    ASSERT_EQ(ret, btree::SUCCESS);

    uint64_t value;
    stream.seekg(offset * sizeof(btree::Node<RECORDS_IN_INDEX_NODE>), std::ios_base::beg);
    stream.read(reinterpret_cast<char *>(&value), sizeof(value));
    EXPECT_EQ(value, node.usage);
    EXPECT_EQ(btree::g_iinfo.writes, 1);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENwriteAfterEndOffsetTHENreturnsErrorCode) {
    auto ret = storage.write_node(nodes_count + 1, &node);
    EXPECT_EQ(ret, btree::INVALID_OFFSET);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENwriteToEndOffsetTHENreturnsSuccessAndProperlyWrites) {
    btree::g_iinfo.reset();
    const uint64_t offset = 3;
    auto ret = storage.write_node(offset, &node);
    ASSERT_EQ(ret, btree::SUCCESS);

    uint64_t value;
    stream.seekg(offset * sizeof(btree::Node<RECORDS_IN_INDEX_NODE>), std::ios_base::beg);
    stream.read(reinterpret_cast<char *>(&value), sizeof(value));
    EXPECT_EQ(value, node.usage);
    EXPECT_EQ(btree::g_iinfo.writes, 1);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENdeleteEndOffsetTHENreturnsErrorCode) {
    auto ret = storage.delete_node(nodes_count);
    EXPECT_EQ(ret, btree::INVALID_OFFSET);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENdeleteAfterEndOffsetTHENreturnsErrorCode) {
    auto ret = storage.delete_node(nodes_count + 1);
    EXPECT_EQ(ret, btree::INVALID_OFFSET);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENdeleteExistingOffsetTHENreturnsSuccess) {
    auto ret = storage.delete_node(1);
    EXPECT_EQ(ret, btree::SUCCESS);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENdeleteFreeOffsetTHENreturnsErrorCode) {
    auto ret = storage.delete_node(1);
    ASSERT_EQ(ret, btree::SUCCESS);

    ret = storage.delete_node(1);
    EXPECT_EQ(ret, btree::INVALID_OFFSET);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENgetFreeOffsetTHENreturnsProperFreeOffset) {
    auto ret = storage.get_free_offset();
    EXPECT_EQ(ret, nodes_count);
}


TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENdeleteExistingOffsetAndGetFreeOffsetTHENreturnsProperFreeOffset) {
    auto ret = storage.delete_node(1);
    ASSERT_EQ(ret, btree::SUCCESS);

    auto offset = storage.get_free_offset();
    EXPECT_EQ(offset, 1);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENreserveIllegalOffsetTHENreturnsErrorCode) {
    auto ret = storage.reserve_offset(nodes_count - 1);
    EXPECT_EQ(ret, btree::INVALID_OFFSET);

    ret = storage.reserve_offset(nodes_count + 1);
    EXPECT_EQ(ret, btree::INVALID_OFFSET);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENreserveLegalOffsetTHENreturnsSuccessAndProperlyReserved) {
    auto ret = storage.reserve_offset(nodes_count);
    EXPECT_EQ(ret, btree::SUCCESS);
    EXPECT_EQ(storage.get_free_offset(), nodes_count + 1);

    ret = storage.delete_node(1);
    ASSERT_EQ(ret, btree::SUCCESS);

    ret = storage.reserve_offset(1);
    EXPECT_EQ(ret, btree::SUCCESS);
    EXPECT_EQ(storage.get_free_offset(), nodes_count + 1);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENgetFreeOffsetWithReserveParamSetTHENreturnsProperFreeOffsetAndReservesIt) {
    auto ret = storage.get_free_offset(true);
    EXPECT_EQ(ret, nodes_count);
    EXPECT_EQ(storage.get_free_offset(), nodes_count + 1);
}

TEST_F(StorageBasicTest, GIVENstorageWithNodesWHENdeleteThenWriteToFreeOffsetTHENproperWriteAndDeletedOffsetFromFree) {
    btree::g_iinfo.reset();
    const uint64_t offset = 1;

    auto ret = storage.delete_node(offset);
    ASSERT_EQ(ret, btree::SUCCESS);
    ASSERT_EQ(storage.get_free_offset(), offset);

    ret = storage.write_node(offset, &node);
    ASSERT_EQ(ret, btree::SUCCESS);
    EXPECT_NE(storage.get_free_offset(), offset);

    uint64_t value;
    stream.seekg(offset * sizeof(btree::Node<RECORDS_IN_INDEX_NODE>), std::ios_base::beg);
    stream.read(reinterpret_cast<char *>(&value), sizeof(value));
    EXPECT_EQ(value, node.usage);
    EXPECT_EQ(btree::g_iinfo.writes, 1);
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