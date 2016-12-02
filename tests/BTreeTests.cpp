#include "gtest/gtest.h"
#include "BTree.h"

struct BTreeBasicTest : public ::testing::Test {
    BTreeBasicTest() : container(3, 3), record(123, "CB12345"), record_updated(123, "GD12345") {};

    btree::Container container;
    btree::Record record, record_updated;
};

struct BTreeAdvancedTest : public ::testing::Test {
    BTreeAdvancedTest() : container(3, 3), record(123, "CB12345"), record2(234, "CB67890"),
                          record3(345, "WI12345"), record4(456, "GD67890"), record5(567, "GD12345") {};

    void SetUp() override {
        auto ret = container.insert(record);
        ret &= container.insert(record2);
        ret &= container.insert(record3);
        ret &= container.insert(record5);
        ret &= container.insert(record4);
        ASSERT_TRUE(ret == btree::SUCCESS);
    }

    btree::Container container;
    btree::Record record, record2, record3, record4, record5;
};

TEST_F(BTreeBasicTest, GIVENnothingWHENcreateContainerWith65IndexesOnPageTHENthrowException) {
    auto check = false;

    try {
        btree::Container test_container(1, 65);
    } catch( int ex_code ) {
        EXPECT_EQ(ex_code, btree::BAD_PARAMETER);
        check = true;
    }

    EXPECT_TRUE(check);
}

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