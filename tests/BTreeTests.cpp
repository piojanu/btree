#include "gtest/gtest.h"
#include "BTree.h"

const uint32_t RECORDS_ON_DATA_PAGE = 3;
const uint8_t RECORDS_IN_INDEX_NODE = 3;

struct BTreeBasicTest : public ::testing::Test {
    BTreeBasicTest() : container(), record(123, "CB12345"),
                       record_updated(123, "GD12345") {};

    btree::Container<RECORDS_ON_DATA_PAGE, RECORDS_IN_INDEX_NODE> container;
    btree::Record record, record_updated;
};

struct BTreeAdvancedTest : public ::testing::Test {
    BTreeAdvancedTest() : container() {};

    void SetUp() override {
        uint64_t key = 1;
        for (auto record : records) {
            record.key = key;
            std::memcpy(record.value, "CB12345", sizeof(record.value));
            key *= 3;

            auto ret = container.insert(record);
            ASSERT_TRUE(ret == btree::SUCCESS);
        }
    }

    btree::Container<RECORDS_ON_DATA_PAGE, RECORDS_IN_INDEX_NODE> container;
    std::array<btree::Record, 5> records;
};

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENinsertTHENreturnSuccess) {
    auto ret = container.insert(record);
    EXPECT_TRUE(ret == btree::SUCCESS);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENupdateTHENreturnSuccess) {
    auto ret = container.insert(record);
    ASSERT_TRUE(ret == btree::SUCCESS);

    ret = container.update(record_updated);
    EXPECT_TRUE(ret == btree::SUCCESS);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENremoveTHENreturnSuccess) {
    auto ret = container.insert(record);
    ASSERT_TRUE(ret == btree::SUCCESS);

    ret = container.remove(record.key);
    EXPECT_TRUE(ret == btree::SUCCESS);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENgetValueTHENreturnSuccessAndproperValue) {
    auto ret = container.insert(record);
    ASSERT_TRUE(ret == btree::SUCCESS);

    std::string value;
    ret = container.get_value(record.key, value);
    EXPECT_TRUE(ret == btree::SUCCESS);
    EXPECT_STREQ(value.c_str(), record.value);
}

TEST_F(BTreeAdvancedTest, GIVENcontainerWith5RecordsWHENprintRawIndexTHENproperBinaryPrint) {
    std::stringstream stream;
    auto ret = container.print_raw_index(stream);
    ASSERT_TRUE(ret == btree::SUCCESS);

    btree::Node<RECORDS_IN_INDEX_NODE> *node = reinterpret_cast<btree::Node<RECORDS_IN_INDEX_NODE> *>(std::malloc(
            sizeof(btree::Node<RECORDS_IN_INDEX_NODE>)));

    // First node, full
    stream.read(reinterpret_cast<char *>(node), sizeof(btree::Node<RECORDS_IN_INDEX_NODE>));
    for (auto i = 0; i < 3; i++) {
        EXPECT_EQ(-1, node->node_entries[i].offset);
        EXPECT_EQ(records[i].key, node->node_entries[i].record.key);
        EXPECT_STREQ(records[i].value, node->node_entries[i].record.value);
    }
    EXPECT_EQ(1, node->offset);

    // Second node, 2 from 3 node entries
    stream.read(reinterpret_cast<char *>(node), sizeof(btree::Node<RECORDS_IN_INDEX_NODE>));
    for (auto i = 0; i < 2; i++) {
        EXPECT_EQ(-1, node->node_entries[i].offset);
        EXPECT_EQ(records[i + 3].key, node->node_entries[i].record.key);
        EXPECT_STREQ(records[i + 3].value, node->node_entries[i].record.value);
    }
    EXPECT_EQ(-1, node->offset);

    std::free(node);
}