#include "gtest/gtest.h"
#include "BTree.h"

const uint32_t RECORDS_IN_INDEX_NODE = 3;

struct BTreeBasicTest : public ::testing::Test {
    BTreeBasicTest() : container(), value("CB12345"), value_updated("GD12345"), key(123) {}

    btree::Container<RECORDS_IN_INDEX_NODE> container;
    char value[8], value_updated[8];
    uint64_t key = 123;
};

struct BTreeAdvancedTest : public ::testing::Test {
    BTreeAdvancedTest() : container(), value("CB12345") {}

    void SetUp() override {
        uint64_t key = records.size();
        for (auto record : records) {
            record.first = key;
            std::strcpy(record.second, value);

            auto ret = container.insert(record.first, record.second);
            ASSERT_TRUE(ret == btree::SUCCESS);

            key -= 1;
        }
    }

    btree::Container<RECORDS_IN_INDEX_NODE> container;
    std::array<std::pair<uint64_t, char[8]>, 5> records;
    const char value[8];
};

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENinsertTHENreturnSuccess) {
    auto ret = container.insert(key, value);
    EXPECT_TRUE(ret == btree::SUCCESS);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENupdateTHENreturnRecordNotFound) {
    auto ret = container.update(key, value_updated);
    EXPECT_TRUE(ret == btree::RECORD_NOT_FOUND);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithRecordWHENupdateRecordTHENreturnSuccess) {
    auto ret = container.insert(key, value);
    ASSERT_TRUE(ret == btree::SUCCESS);

    ret = container.update(key, value_updated);
    EXPECT_TRUE(ret == btree::SUCCESS);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENremoveTHENreturnRecordNotFound) {
    auto ret = container.remove(key);
    EXPECT_TRUE(ret == btree::RECORD_NOT_FOUND);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithRecordWHENremoveRecordTHENreturnSuccess) {
    auto ret = container.insert(key, value);
    ASSERT_TRUE(ret == btree::SUCCESS);

    ret = container.remove(key);
    EXPECT_TRUE(ret == btree::SUCCESS);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENgetValueTHENreturnRecordNotFound) {
    char ret_value[8];
    auto ret = container.get_value(key, ret_value);
    EXPECT_TRUE(ret == btree::RECORD_NOT_FOUND);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithRecordWHENgetValueTHENreturnSuccessAndproperValue) {
    auto ret = container.insert(key, value);
    ASSERT_TRUE(ret == btree::SUCCESS);

    char ret_value[8];
    ret = container.get_value(key, ret_value);
    EXPECT_TRUE(ret == btree::SUCCESS);
    EXPECT_STREQ(value, ret_value);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithRecordWHENupdateRecordAndGetValueTHENreturnSuccessAndproperValue) {
    auto ret = container.insert(key, value);
    ASSERT_TRUE(ret == btree::SUCCESS);

    ret = container.update(key, value_updated);
    ASSERT_TRUE(ret == btree::SUCCESS);

    char ret_value[8];
    ret = container.get_value(key, ret_value);
    EXPECT_TRUE(ret == btree::SUCCESS);
    EXPECT_STREQ(value_updated, ret_value);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithRecordWHENremoveRecordAndGetValueTHENreturnRecordNotFound) {
    auto ret = container.insert(key, value);
    ASSERT_TRUE(ret == btree::SUCCESS);

    ret = container.remove(key);
    ASSERT_TRUE(ret == btree::SUCCESS);

    char ret_value[8];
    ret = container.get_value(key, ret_value);
    EXPECT_TRUE(ret == btree::RECORD_NOT_FOUND);
}

TEST_F(BTreeAdvancedTest, GIVENcontainerWith5RecordsWHENprintDataOrderedTHENproperPrint) {
    std::stringstream stream;
    auto ret = container.print_data_ordered(stream);
    ASSERT_TRUE(ret == btree::SUCCESS);

    for (auto i = static_cast<int64_t >(records.size()) - 1; i >= 0; i--) {
        uint64_t key;
        std::string value;

        stream >> key >> value;
        EXPECT_EQ(records[i].first, key);
        EXPECT_STREQ(records[i].second, value.c_str());
    }
}

TEST_F(BTreeAdvancedTest, GIVENcontainerWith5RecordsWHENprintRawFileTHENproperBinaryPrint) {
    std::stringstream stream;
    auto ret = container.print_raw_file(stream);
    ASSERT_TRUE(ret == btree::SUCCESS);

    uint64_t *data = reinterpret_cast<uint64_t *>(std::malloc(sizeof(uint64_t)));
    char data_ch[8];

    // First node [1 1 2 2 0 0 0 0]
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(1, *data);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(1, *data);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(2, *data);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(2, *data);
    for (auto i = 0; i < 4; i++) {
        stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
        EXPECT_EQ(0, *data);
    }

    // Second node [2 1 "CB12345" 2 "CB12345" 0 0 0]
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(2, *data);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(1, *data);
    stream.read(data_ch, sizeof(data_ch));
    EXPECT_STREQ(value, data_ch);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(2, *data);
    stream.read(data_ch, sizeof(data_ch));
    EXPECT_STREQ(value, data_ch);
    for (auto i = 0; i < 3; i++) {
        stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
        EXPECT_EQ(0, *data);
    }

    // Third node [3 3 "CB12345" 4 "CB12345" 5 "CB12345" 0]
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(3, *data);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(3, *data);
    stream.read(data_ch, sizeof(data_ch));
    EXPECT_STREQ(value, data_ch);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(4, *data);
    stream.read(data_ch, sizeof(data_ch));
    EXPECT_STREQ(value, data_ch);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(5, *data);
    stream.read(data_ch, sizeof(data_ch));
    EXPECT_STREQ(value, data_ch);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(0, *data);

    std::free(data);
}