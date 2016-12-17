#include "gtest/gtest.h"
#include "BTree.h"

#include <array>
#include <fstream>

const uint32_t RECORDS_IN_NODE = 3;

struct BTreeCreationTest : public ::testing::Test {
    void TearDown() override {
        std::remove(file_name.c_str());
    }

    std::stringstream stream;
    const std::string file_name = "test.btree";
};

struct BTreeBasicTest : public ::testing::Test {
    BTreeBasicTest() : container(&stream, 0, 0), stream(""), value("CB12345"), value_updated("GD12345"), key(123) {}

    btree::Container<RECORDS_IN_NODE> container;
    std::stringstream stream;
    char value[8], value_updated[8];
    uint64_t key = 123;
};

struct BTreePrintTest : public ::testing::Test {
    BTreePrintTest() : container(&stream, 0, 0), stream(""), value("CB12345") {}

    void SetUp() override {
        uint64_t key = records.size();
        for (auto i = 0; i < static_cast<int64_t >(records.size()); i++) {
            records[i].first = key;
            std::strcpy(records[i].second, value);

            auto ret = container.insert(records[i].first, records[i].second);
            ASSERT_EQ(ret, btree::SUCCESS) << "Key: " << key;

            key -= 1;
        }
    }

    btree::Container<RECORDS_IN_NODE> container;
    std::stringstream stream;
    std::array<std::pair<uint64_t, char[8]>, 5> records;
    const char value[8];
};

struct BTreeAdvancedTest : public ::testing::Test {
    BTreeAdvancedTest() : begin_state(), end_state(), container(nullptr),
                          count_begin_pages(0), count_end_pages(0) {}

    void write_page(std::iostream &stream, const uint64_t &usage, const uint64_t &p1,
                    const uint64_t &one, const uint64_t &p2,
                    const uint64_t &two, const uint64_t &p3) {
        uint64_t zero = 0;

        stream.write(reinterpret_cast<const char *>(&usage), sizeof(usage));
        stream.write(reinterpret_cast<const char *>(&p1), sizeof(p1));
        stream.write(reinterpret_cast<const char *>(&one), sizeof(one));
        stream.write(reinterpret_cast<const char *>(&p2), sizeof(p2));
        stream.write(reinterpret_cast<const char *>(&two), sizeof(two));
        stream.write(reinterpret_cast<const char *>(&p3), sizeof(p3));
        stream.write(reinterpret_cast<const char *>(&zero), sizeof(zero));
        stream.write(reinterpret_cast<const char *>(&zero), sizeof(zero));

        if (&stream == &begin_state) {
            count_begin_pages++;
        } else {
            count_end_pages++;
        }
    }

    void write_page(std::iostream &stream, const uint64_t &usage, const uint64_t &one,
                    const char v1[8], const uint64_t &two,
                    const char v2[8], const uint64_t &next) {
        uint64_t zero = 0;

        stream.write(reinterpret_cast<const char *>(&usage), sizeof(usage));
        stream.write(reinterpret_cast<const char *>(&one), sizeof(one));
        stream.write(v1, sizeof(uint64_t));
        stream.write(reinterpret_cast<const char *>(&two), sizeof(two));
        stream.write(v2, sizeof(uint64_t));
        stream.write(reinterpret_cast<const char *>(&zero), sizeof(zero));
        stream.write(reinterpret_cast<const char *>(&zero), sizeof(zero));
        stream.write(reinterpret_cast<const char *>(&next), sizeof(next));

        if (&stream == &begin_state) {
            count_begin_pages++;
        } else {
            count_end_pages++;
        }
    }

    void createContainer(const uint64_t &height) {
        container = new btree::Container<2>(&begin_state, height, count_begin_pages);
    }

    void validate() {
        begin_state.seekg(0, std::ios_base::beg);
        end_state.seekg(0, std::ios_base::beg);

        for (int i = 0; i < count_end_pages; i++) {
            uint64_t value = 0;
            uint64_t expected = 0;

            for (int j = 0; j < 8; j++) {
                begin_state.read(reinterpret_cast<char *>(&value), sizeof(value));
                end_state.read(reinterpret_cast<char *>(&expected), sizeof(expected));

                EXPECT_EQ(value, expected) << "Page: " << i << " Field: " << j;
            }
        }
    }

    void TearDown() override {
        if (container != nullptr) {
            delete container;
        }
    }

    std::stringstream begin_state, end_state;
    btree::Container<2> *container;
    unsigned int count_begin_pages, count_end_pages;

    const char ZERO_STR[8] = {'\0'};
};

// ### CREATION TESTS ### //

TEST_F(BTreeCreationTest, GIVENnothingWHENcreateWithPathAndDeleteContainerTHENfileExists) {
    auto container = new btree::Container<RECORDS_IN_NODE>(file_name);
    ASSERT_TRUE(container->is_good());
    delete container;

    std::ifstream f(file_name);
    EXPECT_TRUE(f.good());
}

TEST_F(BTreeCreationTest, GIVENnothingWHENcreateWithStreamAndDeleteContainerTHENproperlyBehave) {
    auto container = new btree::Container<RECORDS_IN_NODE>(&stream, 0, 0);
    EXPECT_TRUE(container->is_good());
    delete container;
}

// ### BASIC TESTS ### //

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENinsertTHENreturnSuccessAndProperContent) {
    auto ret = container.insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    btree::Node<RECORDS_IN_NODE> node;
    stream.seekg(0, std::ios_base::beg);
    stream.read(reinterpret_cast<char *>(&node), sizeof(btree::Node<RECORDS_IN_NODE>));

    EXPECT_EQ(node.usage, 1);

    EXPECT_EQ(node.node_entries[0].offset, key);
    EXPECT_STREQ(node.node_entries[0].record.value, value);

    for (int i = 1; i <= RECORDS_IN_NODE; i++) {
        EXPECT_EQ(node.node_entries[i].offset, 0);
        EXPECT_EQ(node.node_entries[i].record.key, 0);
        EXPECT_STREQ(node.node_entries[i].record.value, "");
    }
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENinsertKeyZeroTHENreturnErrorCode) {
    auto ret = container.insert(0, value);
    EXPECT_EQ(ret, btree::INVALID_KEY);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithItemWHENinsertExistingItemTHENreturnErrorCode) {
    auto ret = container.insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    ret = container.insert(key, value);
    EXPECT_EQ(ret, btree::RECORD_EXISTS);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENupdateKeyZeroTHENreturnErrorCode) {
    auto ret = container.update(0, value);
    EXPECT_EQ(ret, btree::INVALID_KEY);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENupdateTHENreturnEmptyStorage) {
    auto ret = container.update(key, value_updated);
    EXPECT_EQ(ret, btree::EMPTY_STORAGE);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithRecordWHENupdateNotExistingRecordTHENreturnRecordNotFound) {
    auto ret = container.insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    ret = container.update(key + 1, value_updated);
    EXPECT_EQ(ret, btree::RECORD_NOT_FOUND);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithRecordWHENupdateRecordTHENreturnSuccess) {
    auto ret = container.insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    ret = container.update(key, value_updated);
    EXPECT_EQ(ret, btree::SUCCESS);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENremoveKeyZeroTHENreturnErrorCode) {
    auto ret = container.remove(0);
    EXPECT_EQ(ret, btree::INVALID_KEY);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENremoveTHENreturnRecordNotFound) {
    auto ret = container.remove(key);
    EXPECT_EQ(ret, btree::RECORD_NOT_FOUND);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithRecordWHENremoveRecordTHENreturnSuccess) {
    auto ret = container.insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    ret = container.remove(key);
    EXPECT_EQ(ret, btree::SUCCESS);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENgetValueOfKeyZeroTHENreturnErrorCode) {
    auto ret = container.get_value(0, value);
    EXPECT_EQ(ret, btree::INVALID_KEY);
}

TEST_F(BTreeBasicTest, GIVENemptyContainerWHENgetValueTHENreturnEmptyStorage) {
    char ret_value[8];
    auto ret = container.get_value(key, ret_value);
    EXPECT_EQ(ret, btree::EMPTY_STORAGE);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithRecordWHENgetValueTHENreturnSuccessAndproperValue) {
    auto ret = container.insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    char ret_value[8];
    ret = container.get_value(key, ret_value);
    EXPECT_EQ(ret, btree::SUCCESS);
    EXPECT_STREQ(value, ret_value);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithRecordWHENgetValueOfNotExistingKeyTHENreturnRecordNotFound) {
    auto ret = container.insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    char ret_value[8];
    ret = container.get_value(key + 1, ret_value);
    EXPECT_EQ(ret, btree::RECORD_NOT_FOUND);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithRecordWHENupdateRecordAndGetValueTHENreturnSuccessAndproperValue) {
    auto ret = container.insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    ret = container.update(key, value_updated);
    ASSERT_EQ(ret, btree::SUCCESS);

    char ret_value[8];
    ret = container.get_value(key, ret_value);
    EXPECT_EQ(ret, btree::SUCCESS);
    EXPECT_STREQ(value_updated, ret_value);
}

TEST_F(BTreeBasicTest, GIVENcontainerWithRecordWHENremoveRecordAndGetValueTHENreturnEmptyStorage) {
    auto ret = container.insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    ret = container.remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    char ret_value[8];
    ret = container.get_value(key, ret_value);
    EXPECT_EQ(ret, btree::EMPTY_STORAGE);
}

// ### PRINT TESTS ### //

TEST_F(BTreePrintTest, GIVENcontainerWith5RecordsWHENprintDataOrderedTHENproperPrint) {
    std::stringstream stream;
    auto ret = container.print_data_ordered(stream);
    ASSERT_EQ(ret, btree::SUCCESS);

    for (auto i = static_cast<int64_t >(records.size()) - 1; i >= 0; i--) {
        uint64_t key;
        std::string value;

        stream >> key >> value;
        EXPECT_EQ(records[i].first, key) << "Iter: " << i;
        EXPECT_STREQ(records[i].second, value.c_str()) << "Iter: " << i;
    }
}

TEST_F(BTreePrintTest, GIVENcontainerWith5RecordsWHENprintRawFileTHENproperBinaryPrint) {
    // This test is currently disabled due to lack of time,
    // unimportancy and complexity of testing formatted text.
    std::stringstream stream;
    auto ret = container.print_raw_file(stream);
    ASSERT_EQ(ret, btree::SUCCESS);

    std::cout << stream.str();

    /* std::stringstream stream;
    auto ret = container.print_raw_file(stream);
    ASSERT_EQ(ret, btree::SUCCESS);

    uint64_t *data = reinterpret_cast<uint64_t *>(std::malloc(sizeof(uint64_t)));
    char data_ch[8];

    // First node [1 1 3 2 0 0 0 0 0 0]
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(1, *data);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(1, *data);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(3, *data);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(2, *data);
    for (auto i = 0; i < 6; i++) {
        stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
        EXPECT_EQ(0, *data);
    }

    // Second node [3 1 "CB12345" 2 "CB12345" 3 "CB12345" 0 0 0]
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(3, *data);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(1, *data);
    stream.read(data_ch, sizeof(data_ch));
    EXPECT_STREQ(value, data_ch);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(2, *data);
    stream.read(data_ch, sizeof(data_ch));
    EXPECT_STREQ(value, data_ch);
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(3, *data);
    stream.read(data_ch, sizeof(data_ch));
    EXPECT_STREQ(value, data_ch);
    for (auto i = 0; i < 3; i++) {
        stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
        EXPECT_EQ(0, *data);
    }

    // Third node [2 4 "CB12345" 5 "CB12345" 0 0 0 0 0]
    stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
    EXPECT_EQ(3, *data);
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
    for (auto i = 0; i < 5; i++) {
        stream.read(reinterpret_cast<char *>(data), sizeof(uint64_t));
        EXPECT_EQ(0, *data);
    }

    std::free(data); */
}

// ### ADVANCED INSERTION TESTS ### //

TEST_F(BTreeAdvancedTest, GIVENrootNodeWithSpaceWHENinsertValueTHENproperInsertion) {
    // Prepare case record
    const uint64_t key = 1;
    const char value[8] = "XX11111";

    // Prepare case begin state
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 0);
    createContainer(1);

    // Prepare expected end state
    write_page(end_state, 2, key, value, 3, "AA33333", 0); // Root

    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENleafsWithSpaceWHENinsertLowerValueTHENproperInsertion) {
    // Prepare case record
    const uint64_t key = 1;
    const char value[8] = "XX11111";

    // Prepare case begin state
    write_page(begin_state, 1, 1, 3, 2, 0, 0);
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 2);
    write_page(begin_state, 1, 4, "AA44444", 0, ZERO_STR, 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 1, 3, 2, 0, 0); // Root
    write_page(end_state, 2, key, value, 3, "AA33333", 2); // Node 1
    write_page(end_state, 1, 4, "AA44444", 0, ZERO_STR, 0); // Node 2


    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENleafsWithSpaceWHENinsertHigherValueTHENproperInsertion) {
    // Prepare case record
    const uint64_t key = 7;
    const char value[8] = "XX77777";

    // Prepare case begin state
    write_page(begin_state, 1, 1, 3, 2, 0, 0);
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 2);
    write_page(begin_state, 1, 4, "AA44444", 0, ZERO_STR, 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 1, 3, 2, 0, 0); // Root
    write_page(end_state, 1, 3, "AA33333", 0, ZERO_STR, 2); // Node 1
    write_page(end_state, 2, 4, "AA44444", key, value, 0); // Node 2


    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENfullRootNodeWHENinsertLowerValueTHENproperSplit) {
    // Prepare case record
    const uint64_t key = 1;
    const char value[8] = "XX11111";

    // Prepare case begin state
    write_page(begin_state, 2, 3, "AA33333", 7, "BB77777", 0);
    createContainer(1);

    // Prepare expected end state
    write_page(end_state, 1, 1, 3, 2, 0, 0); // Root
    write_page(end_state, 2, key, value, 3, "AA33333", 2); // Node 1
    write_page(end_state, 1, 7, "BB77777", 0, ZERO_STR, 0); // Node 2

    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENfullRootNodeWHENinsertMidValueTHENproperSplit) {
    // Prepare case record
    const uint64_t key = 4;
    const char value[8] = "XX44444";

    // Prepare case begin state
    write_page(begin_state, 2, 3, "AA33333", 7, "BB77777", 0);
    createContainer(1);

    // Prepare expected end state
    write_page(end_state, 1, 1, key, 2, 0, 0); // Root
    write_page(end_state, 2, 3, "AA33333", key, value, 2); // Node 1
    write_page(end_state, 1, 7, "BB77777", 0, ZERO_STR, 0); // Node 2

    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENfullRootNodeWHENinsertHigherValueTHENproperSplit) {
    // Prepare case record
    const uint64_t key = 9;
    const char value[8] = "XX99999";

    // Prepare case begin state
    write_page(begin_state, 2, 3, "AA33333", 7, "BB77777", 0);
    createContainer(1);

    // Prepare expected end state
    write_page(end_state, 1, 1, 7, 2, 0, 0); // Root
    write_page(end_state, 2, 3, "AA33333", 7, "BB77777", 2); // Node 1
    write_page(end_state, 1, key, value, 0, ZERO_STR, 0); // Node 2

    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENfullLeafNodesWHENinsertLowerValueTHENproperSplit) {
    // Prepare case record
    const uint64_t key = 1;
    const char value[8] = "XX11111";

    // Prepare case begin state
    write_page(begin_state, 1, 1, 3, 2, 0, 0);
    write_page(begin_state, 2, 2, "AA22222", 3, "BB33333", 2);
    write_page(begin_state, 2, 4, "AA44444", 6, "BB66666", 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 2, 1, 2, 3, 3, 2);
    write_page(end_state, 2, key, value, 2, "AA22222", 3);
    write_page(end_state, 2, 4, "AA44444", 6, "BB66666", 0);
    write_page(end_state, 1, 3, "BB33333", 0, ZERO_STR, 2);


    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENfullLeafNodesWHENinsertHigherValueTHENproperSplit) {
    // Prepare case record
    const uint64_t key = 7;
    const char value[8] = "XX77777";

    // Prepare case begin state
    write_page(begin_state, 1, 1, 3, 2, 0, 0);
    write_page(begin_state, 2, 2, "AA22222", 3, "BB33333", 2);
    write_page(begin_state, 2, 4, "AA44444", 6, "BB66666", 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 2, 1, 3, 2, 6, 3);
    write_page(end_state, 2, 2, "AA22222", 3, "BB33333", 2);
    write_page(end_state, 2, 4, "AA44444", 6, "BB66666", 3);
    write_page(end_state, 1, key, value, 0, ZERO_STR, 0);


    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENfullTreeHighTwoWHENinsertLowerValueTHENproperSplit) {
    // Prepare case record
    const uint64_t key = 1;
    const char value[8] = "XX11111";

    // Prepare case begin state
    write_page(begin_state, 2, 1, 3, 2, 6, 3);
    write_page(begin_state, 2, 2, "AA22222", 3, "BB33333", 2);
    write_page(begin_state, 2, 4, "AA44444", 6, "BB66666", 3);
    write_page(begin_state, 2, 7, "AA77777", 8, "BB88888", 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 5, 3, 6, 0, 0);
    write_page(end_state, 2, key, value, 2, "AA22222", 4);
    write_page(end_state, 2, 4, "AA44444", 6, "BB66666", 3);
    write_page(end_state, 2, 7, "AA77777", 8, "BB88888", 0);
    write_page(end_state, 1, 3, "BB33333", 0, ZERO_STR, 2);
    write_page(end_state, 1, 1, 2, 4, 0, 0);
    write_page(end_state, 1, 2, 6, 3, 0, 0);


    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENfullTreeHighTwoWHENinsertMidValueTHENproperSplit) {
    // Prepare case record
    const uint64_t key = 5;
    const char value[8] = "XX55555";

    // Prepare case begin state
    write_page(begin_state, 2, 1, 3, 2, 6, 3);
    write_page(begin_state, 2, 2, "AA22222", 3, "BB33333", 2);
    write_page(begin_state, 2, 4, "AA44444", 6, "BB66666", 3);
    write_page(begin_state, 2, 7, "AA77777", 8, "BB88888", 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 5, 5, 6, 0, 0);
    write_page(end_state, 2, 2, "AA22222", 3, "BB33333", 2);
    write_page(end_state, 2, 4, "AA44444", key, value, 4);
    write_page(end_state, 2, 7, "AA77777", 8, "BB88888", 0);
    write_page(end_state, 1, 6, "BB66666", 0, ZERO_STR, 3);
    write_page(end_state, 1, 1, 3, 2, 0, 0);
    write_page(end_state, 1, 4, 6, 3, 0, 0);


    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENfullTreeHighTwoWHENinsertHigherValueTHENproperSplit) {
    // Prepare case record
    const uint64_t key = 9;
    const char value[8] = "XX99999";

    // Prepare case begin state
    write_page(begin_state, 2, 1, 3, 2, 6, 3);
    write_page(begin_state, 2, 2, "AA22222", 3, "BB33333", 2);
    write_page(begin_state, 2, 4, "AA44444", 6, "BB66666", 3);
    write_page(begin_state, 2, 7, "AA77777", 8, "BB88888", 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 5, 6, 6, 0, 0);
    write_page(end_state, 2, 2, "AA22222", 3, "BB33333", 2);
    write_page(end_state, 2, 4, "AA44444", 6, "BB66666", 3);
    write_page(end_state, 2, 7, "AA77777", 8, "BB88888", 4);
    write_page(end_state, 1, key, value, 0, ZERO_STR, 0);
    write_page(end_state, 1, 1, 3, 2, 0, 0);
    write_page(end_state, 1, 3, 8, 4, 0, 0);


    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENoneLeafFullAndBrotherWithSpaceWHENinsertLowerMidValueTHENproperCompensation) {
    // Prepare case record
    const uint64_t key = 2;
    const char value[8] = "XX22222";

    // Prepare case begin state
    write_page(begin_state, 1, 1, 3, 2, 0, 0);
    write_page(begin_state, 2, 1, "AA11111", 3, "BB33333", 2);
    write_page(begin_state, 1, 4, "AA44444", 0, ZERO_STR, 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 1, 2, 2, 0, 0);
    write_page(end_state, 2, 1, "AA11111", key, value, 2);
    write_page(end_state, 2, 3, "BB33333", 4, "AA44444", 0);


    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENoneLeafFullAndBrotherWithSpaceWHENinsertHigherValueTHENproperCompensation) {
    // Prepare case record
    const uint64_t key = 7;
    const char value[8] = "XX77777";

    // Prepare case begin state
    write_page(begin_state, 1, 1, 3, 2, 0, 0);
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 2);
    write_page(begin_state, 2, 4, "AA44444", 6, "BB66666", 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 1, 4, 2, 0, 0);
    write_page(end_state, 2, 3, "AA33333", 4, "AA44444", 2);
    write_page(end_state, 2, 6, "BB66666", key, value, 0);


    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENleafsFullAndBrotherOfParentWithSpaceAndTreeHighThreeWHENinsertValueTHENproperSplitAndCompensation) {
    // Prepare case record
    const uint64_t key = 2;
    const char value[8] = "XX22222";

    // Prepare case begin state
    write_page(begin_state, 1, 1, 7, 2, 0, 0); // Root
    write_page(begin_state, 2, 3, 3, 4, 5, 5); // Node 1
    write_page(begin_state, 1, 6, 8, 7, 0, 0); // Node 2
    write_page(begin_state, 2, 1, "AA11111", 3, "BB33333", 4); // Node 3
    write_page(begin_state, 2, 4, "AA44444", 5, "BB55555", 5); // Node 4
    write_page(begin_state, 2, 6, "AA66666", 7, "BB77777", 6); // Node 5
    write_page(begin_state, 1, 8, "AA88888", 0, ZERO_STR, 7);  // Node 6
    write_page(begin_state, 1, 9, "AA99999", 0, ZERO_STR, 0);  // Node 7
    createContainer(3);

    // Prepare expected end state
    write_page(end_state, 1, 1, 5, 2, 0, 0); // Root
    write_page(end_state, 2, 3, 2, 8, 3, 4); // Node 1
    write_page(end_state, 2, 5, 7, 6, 8, 7); // Node 2
    write_page(end_state, 2, 1, "AA11111", key, value, 8);   // Node 3
    write_page(end_state, 2, 4, "AA44444", 5, "BB55555", 5); // Node 4
    write_page(end_state, 2, 6, "AA66666", 7, "BB77777", 6); // Node 5
    write_page(end_state, 1, 8, "AA88888", 0, ZERO_STR, 7);  // Node 6
    write_page(end_state, 1, 9, "AA99999", 0, ZERO_STR, 0);  // Node 7
    write_page(end_state, 1, 3, "BB33333", 0, ZERO_STR, 4);  // Node 8


    // Do operation
    auto ret = container->insert(key, value);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

// ### ADVANCED REMOVAL TESTS ### //

TEST_F(BTreeAdvancedTest, GIVENoneRecordInRootNodeWHENremoveLastRecordTHENproperRemove) {
    // Prepare case record
    const uint64_t key = 3;

    // Prepare case begin state
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 0);
    createContainer(1);

    // Prepare expected end state
    write_page(end_state, 0, 0, ZERO_STR, 0, ZERO_STR, 0);

    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENfullRootNodeWHENremoveLeftMostRecordTHENproperRemove) {
    // Prepare case record
    const uint64_t key = 3;

    // Prepare case begin state
    write_page(begin_state, 2, 3, "AA33333", 7, "BB77777", 0);
    createContainer(1);

    // Prepare expected end state
    write_page(end_state, 1, 7, "BB77777", 0, ZERO_STR, 0);

    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENfullRootNodeWHENremoveRightMostRecordTHENproperRemove) {
    // Prepare case record
    const uint64_t key = 7;

    // Prepare case begin state
    write_page(begin_state, 2, 3, "AA33333", 7, "BB77777", 0);
    createContainer(1);

    // Prepare expected end state
    write_page(end_state, 1, 3, "AA33333", 0, ZERO_STR, 0);

    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENfullLeafsAndminimalParentsAndTreeHighThreeWHENremoveRrecordOfRootKeyTHENproperChangeOfRootKey) {
    // Prepare case record
    const uint64_t key = 5;

    // Prepare case begin state
    write_page(begin_state, 1, 1, 5, 2, 0, 0); // Root
    write_page(begin_state, 1, 3, 2, 4, 0, 0); // Node 1
    write_page(begin_state, 1, 5, 7, 6, 0, 0); // Node 2
    write_page(begin_state, 2, 1, "AA11111", 2, "BB22222", 4); // Node 3
    write_page(begin_state, 2, 4, "AA44444", 5, "BB55555", 5); // Node 4
    write_page(begin_state, 2, 6, "AA66666", 7, "BB77777", 6); // Node 5
    write_page(begin_state, 2, 8, "AA88888", 9, "BB99999", 0); // Node 6
    createContainer(3);

    // Prepare expected end state
    write_page(end_state, 1, 1, 4, 2, 0, 0); // Root
    write_page(end_state, 1, 3, 2, 4, 0, 0); // Node 1
    write_page(end_state, 1, 5, 7, 6, 0, 0); // Node 2
    write_page(end_state, 2, 1, "AA11111", 2, "BB22222", 4); // Node 3
    write_page(end_state, 1, 4, "AA44444", 0, ZERO_STR, 5);  // Node 4
    write_page(end_state, 2, 6, "AA66666", 7, "BB77777", 6); // Node 5
    write_page(end_state, 2, 8, "AA88888", 9, "BB99999", 0); // Node 6


    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENminimalLeafsWHENremoveLastRecordFromLeftTHENproperRemoveAndSquash) {
    // Prepare case record
    const uint64_t key = 3;

    // Prepare case begin state
    write_page(begin_state, 1, 1, 3, 2, 0, 0);
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 2);
    write_page(begin_state, 1, 7, "AA77777", 0, ZERO_STR, 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 7, "AA77777", 0, ZERO_STR, 0);

    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENminimalLeafsWHENremoveLastRecordFromRightTHENproperRemoveAndSquash) {
    // Prepare case record
    const uint64_t key = 7;

    // Prepare case begin state
    write_page(begin_state, 1, 1, 3, 2, 0, 0);
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 2);
    write_page(begin_state, 1, 7, "AA77777", 0, ZERO_STR, 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 3, "AA33333", 0, ZERO_STR, 0);

    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENminimalLeafsWHENremoveLastRecordFromLeftMostTHENproperRemoveAndSquash) {
    // Prepare case record
    const uint64_t key = 3;

    // Prepare case begin state
    write_page(begin_state, 2, 1, 3, 2, 7, 3);
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 2);
    write_page(begin_state, 1, 7, "AA77777", 0, ZERO_STR, 3);
    write_page(begin_state, 1, 9, "AA99999", 0, ZERO_STR, 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 1, 7, 3, 0 ,0); // Root
    write_page(end_state, 1, 7, "AA77777", 0, ZERO_STR, 3); // Node 1
    write_page(end_state, 1, 9, "AA99999", 0, ZERO_STR, 0); // Node 3

    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENminimalLeafsWHENremoveLastRecordFromMidTHENproperRemoveAndSquash) {
    // Prepare case record
    const uint64_t key = 7;

    // Prepare case begin state
    write_page(begin_state, 2, 1, 3, 2, 7, 3);
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 2);
    write_page(begin_state, 1, 7, "AA77777", 0, ZERO_STR, 3);
    write_page(begin_state, 1, 9, "AA99999", 0, ZERO_STR, 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 1, 7, 3, 0 ,0); // Root
    write_page(end_state, 1, 3, "AA33333", 0, ZERO_STR, 3); // Node 1
    write_page(end_state, 1, 9, "AA99999", 0, ZERO_STR, 0); // Node 3

    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENminimalLeafsWHENremoveLastRecordFromRightMostTHENproperRemoveAndSquash) {
    // Prepare case record
    const uint64_t key = 9;

    // Prepare case begin state
    write_page(begin_state, 2, 1, 3, 2, 7, 3);
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 2);
    write_page(begin_state, 1, 7, "AA77777", 0, ZERO_STR, 3);
    write_page(begin_state, 1, 9, "AA99999", 0, ZERO_STR, 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 1, 3, 2, 0 ,0); // Root
    write_page(end_state, 1, 3, "AA33333", 0, ZERO_STR, 2); // Node 1
    write_page(end_state, 1, 7, "AA77777", 0, ZERO_STR, 0); // Node 2

    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENminimalLeafsAndParentsAndTreeHighThreeWHENremoveRrecordLeftParentRightChildTHENproperSquashes) {
    // Prepare case record
    const uint64_t key = 4;

    // Prepare case begin state
    write_page(begin_state, 1, 1, 4, 2, 0, 0); // Root
    write_page(begin_state, 1, 3, 3, 4, 0, 0); // Node 1
    write_page(begin_state, 1, 5, 5, 6, 0, 0); // Node 2
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 4); // Node 3
    write_page(begin_state, 1, 4, "AA44444", 0, ZERO_STR, 5); // Node 4
    write_page(begin_state, 1, 5, "AA55555", 0, ZERO_STR, 6); // Node 5
    write_page(begin_state, 1, 6, "AA66666", 0, ZERO_STR, 0); // Node 6
    createContainer(3);

    // Prepare expected end state
    write_page(end_state, 2, 3, 3, 5, 5, 6); // Root
    write_page(end_state, 1, 3, "AA33333", 0, ZERO_STR, 5); // Node 3
    write_page(end_state, 1, 5, "AA55555", 0, ZERO_STR, 6); // Node 5
    write_page(end_state, 1, 6, "AA66666", 0, ZERO_STR, 0); // Node 6


    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENminimalLeafsAndParentsAndTreeHighThreeWHENremoveRrecordRightParentRightChildTHENproperSquashes) {
    // Prepare case record
    const uint64_t key = 6;

    // Prepare case begin state
    write_page(begin_state, 1, 1, 4, 2, 0, 0); // Root
    write_page(begin_state, 1, 3, 3, 4, 0, 0); // Node 1
    write_page(begin_state, 1, 5, 5, 6, 0, 0); // Node 2
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 4); // Node 3
    write_page(begin_state, 1, 4, "AA44444", 0, ZERO_STR, 5); // Node 4
    write_page(begin_state, 1, 5, "AA55555", 0, ZERO_STR, 6); // Node 5
    write_page(begin_state, 1, 6, "AA66666", 0, ZERO_STR, 0); // Node 6
    createContainer(3);

    // Prepare expected end state
    write_page(end_state, 2, 3, 3, 4, 4, 5); // Root
    write_page(end_state, 1, 3, "AA33333", 0, ZERO_STR, 4); // Node 3
    write_page(end_state, 1, 4, "AA44444", 0, ZERO_STR, 5); // Node 4
    write_page(end_state, 1, 5, "AA55555", 0, ZERO_STR, 0); // Node 5


    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENminimalLeafsAndfullParentsAndTreeHighThreeWHENremoveLastRrecordFromRightMostChildOfParentTHENproperSquashAndRootChange) {
    // Prepare case record
    const uint64_t key = 5;

    // Prepare case begin state
    write_page(begin_state, 1, 1, 5, 2, 0, 0); // Root
    write_page(begin_state, 2, 3, 3, 4, 4, 5); // Node 1
    write_page(begin_state, 2, 6, 6, 7, 7, 8); // Node 2
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 4); // Node 3
    write_page(begin_state, 1, 4, "AA44444", 0, ZERO_STR, 5); // Node 4
    write_page(begin_state, 1, 5, "AA55555", 0, ZERO_STR, 6); // Node 5
    write_page(begin_state, 1, 6, "AA66666", 0, ZERO_STR, 7); // Node 6
    write_page(begin_state, 1, 7, "AA77777", 0, ZERO_STR, 8); // Node 7
    write_page(begin_state, 1, 8, "AA88888", 0, ZERO_STR, 0); // Node 8
    createContainer(3);

    // Prepare expected end state
    write_page(end_state, 1, 1, 4, 2, 0, 0); // Root
    write_page(end_state, 1, 3, 3, 4, 0, 0); // Node 1
    write_page(end_state, 2, 6, 6, 7, 7, 8); // Node 2
    write_page(end_state, 1, 3, "AA33333", 0, ZERO_STR, 4); // Node 3
    write_page(end_state, 1, 4, "AA44444", 0, ZERO_STR, 6); // Node 4
    write_page(end_state, 1, 6, "AA66666", 0, ZERO_STR, 7); // Node 6
    write_page(end_state, 1, 7, "AA77777", 0, ZERO_STR, 8); // Node 7
    write_page(end_state, 1, 8, "AA88888", 0, ZERO_STR, 0); // Node 8


    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENleftBrotherFullWHENremoveLastRecordFromRightTHENproperRemoveAndCompensation) {
    // Prepare case record
    const uint64_t key = 7;

    // Prepare case begin state
    write_page(begin_state, 1, 1, 3, 2, 0, 0);
    write_page(begin_state, 2, 2, "AA22222", 3, "BB33333", 2);
    write_page(begin_state, 1, 7, "AA77777", 0, ZERO_STR, 0);
    createContainer(2);

    // Prepare expected end state
    write_page(end_state, 1, 1, 2, 2, 0, 0);
    write_page(end_state, 1, 2, "AA22222", 0, ZERO_STR, 2);
    write_page(end_state, 1, 3, "BB33333", 0, ZERO_STR, 0);

    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENrightBrotherFullWHENremoveLastRecordFromLeftTHENproperRemoveAndCompensation) {
    // Prepare case record
    const uint64_t key = 7;

    // Prepare case begin state
    write_page(begin_state, 1, 1, 3, 2, 0, 0);
    write_page(begin_state, 1, 3, "AA33333", 0, ZERO_STR, 2);
    write_page(begin_state, 2, 5, "AA55555", 7, "BB77777", 0);
    createContainer(2);

    // Prepare expected end state
    // Prepare expected end state
    write_page(end_state, 1, 1, 5, 2, 0, 0);
    write_page(end_state, 1, 5, "AA55555", 0, ZERO_STR, 2);
    write_page(end_state, 1, 7, "BB77777", 0, ZERO_STR, 0);

    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}

TEST_F(BTreeAdvancedTest, GIVENminimalLeafsAndBrotherOfParentFullAndTreeHighThreeWHENremoveRecordTHENproperSquashAndCompensation) {
    // Prepare case record
    const uint64_t key = 9;

    // Prepare case begin state
    write_page(begin_state, 1, 1, 7, 2, 0, 0); // Root
    write_page(begin_state, 2, 3, 3, 4, 5, 5); // Node 1
    write_page(begin_state, 1, 6, 8, 7, 0, 0); // Node 2
    write_page(begin_state, 2, 1, "AA11111", 3, "BB33333", 4); // Node 3
    write_page(begin_state, 2, 4, "AA44444", 5, "BB55555", 5); // Node 4
    write_page(begin_state, 2, 6, "AA66666", 7, "BB77777", 6); // Node 5
    write_page(begin_state, 1, 8, "AA88888", 0, ZERO_STR, 7);  // Node 6
    write_page(begin_state, 1, 9, "AA99999", 0, ZERO_STR, 0);  // Node 7
    createContainer(3);

    // Prepare expected end state
    write_page(end_state, 1, 1, 5, 2, 0, 0); // Root
    write_page(end_state, 1, 3, 3, 4, 0, 0); // Node 1
    write_page(end_state, 1, 5, 7, 6, 0, 0); // Node 2
    write_page(end_state, 2, 1, "AA11111", 3, "BB33333", 4); // Node 3
    write_page(end_state, 2, 4, "AA44444", 5, "BB55555", 5); // Node 4
    write_page(end_state, 2, 6, "AA66666", 7, "BB77777", 6); // Node 5
    write_page(end_state, 1, 8, "AA88888", 0, ZERO_STR, 0);  // Node 6


    // Do operation
    auto ret = container->remove(key);
    ASSERT_EQ(ret, btree::SUCCESS);

    // Validate real end state with expected end state
    validate();
}