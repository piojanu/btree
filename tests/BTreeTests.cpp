#include "gtest/gtest.h"
#include "BTree.h"

struct BTreeBasicTest : public ::testing::Test {
    BTreeBasicTest() : container(3), record(123, "CB12345"), record_updated(123, "GD12345") {};

    btree::Container container;
    btree::record record, record_updated;
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

    ret = container.remove(record.first);
    EXPECT_TRUE(ret == btree::SUCCESS);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENgetValueTHENreturnSuccessAndproperValue) {
    auto ret = container.insert(record);
    ASSERT_TRUE(ret == btree::SUCCESS);

    std::string value;
    ret = container.get_value(record.first, value);
    EXPECT_TRUE(ret == btree::SUCCESS);
    EXPECT_EQ(value, record.second);
}