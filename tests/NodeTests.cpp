#include "gtest/gtest.h"
#include "Node.h"

struct NodeEntryBasicTest : public ::testing::Test {
    NodeEntryBasicTest() : entry(1, btree::Record(123, "CB12345")), empty_entry() {};

    btree::NodeEntry entry, empty_entry;
};

struct NodeBasicTest : public ::testing::Test {
    NodeBasicTest() : empty_node() {};

    btree::Node<2> empty_node;
};

TEST_F(NodeEntryBasicTest, GIVENcreatedNodeEntryWHENcheckValuesTHENproperValues) {
    EXPECT_EQ(1, entry.offset);
    EXPECT_EQ(123, entry.record.key);
    EXPECT_STREQ("CB12345", entry.record.value);
}

TEST_F(NodeEntryBasicTest, GIVENemptyNodeEntryWHENcheckValuesTHENproperValues) {
    EXPECT_EQ(-1, empty_entry.offset);
    EXPECT_EQ(0, empty_entry.record.key);
    EXPECT_STREQ("", empty_entry.record.value);
}

TEST_F(NodeBasicTest, GIVENemptyNodeWHENcheckValuesTHENproperValues) {
    EXPECT_EQ(-1, empty_node.node_entries[0].offset);
    EXPECT_EQ(0, empty_node.node_entries[0].record.key);
    EXPECT_STREQ("", empty_node.node_entries[0].record.value);

    EXPECT_EQ(-1, empty_node.node_entries[1].offset);
    EXPECT_EQ(0, empty_node.node_entries[1].record.key);
    EXPECT_STREQ("", empty_node.node_entries[1].record.value);

    EXPECT_EQ(-1, empty_node.offset);
}