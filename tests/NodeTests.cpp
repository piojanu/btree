#include "gtest/gtest.h"
#include "Node.h"

struct NodeEntryBasicTest : public ::testing::Test {
    NodeEntryBasicTest() : inner_entry(1, 123), leaf_entry(123, "CB12345"),
                           empty_entry() {}

    btree::NodeEntry inner_entry, leaf_entry, empty_entry;
};

struct NodeBasicTest : public ::testing::Test {
    NodeBasicTest() : empty_node() {};

    btree::Node<2> empty_node;
};

TEST_F(NodeEntryBasicTest, GIVENcreatedInnerNodeEntryWHENcheckValuesTHENproperValues) {
    EXPECT_EQ(1, inner_entry.offset);
    EXPECT_EQ(123, inner_entry.record.key);
}

TEST_F(NodeEntryBasicTest, GIVENcreatedLeafNodeEntryWHENcheckValuesTHENproperValues) {
    EXPECT_EQ(123, leaf_entry.offset);
    EXPECT_STREQ("CB12345", leaf_entry.record.value);
}

TEST_F(NodeEntryBasicTest, GIVENemptyNodeEntryWHENcheckValuesTHENproperValues) {
    EXPECT_EQ(0, empty_entry.offset);
    EXPECT_EQ(0, empty_entry.record.key);
    EXPECT_STREQ("", empty_entry.record.value);
}

TEST_F(NodeBasicTest, GIVENemptyNodeWHENcheckValuesTHENproperValues) {
    EXPECT_EQ(0, empty_node.usage);

    EXPECT_EQ(0, empty_node.node_entries[0].offset);
    EXPECT_EQ(0, empty_node.node_entries[0].record.key);
    EXPECT_STREQ("", empty_node.node_entries[0].record.value);

    EXPECT_EQ(0, empty_node.node_entries[1].offset);
    EXPECT_EQ(0, empty_node.node_entries[1].record.key);
    EXPECT_STREQ("", empty_node.node_entries[1].record.value);

    EXPECT_EQ(0, empty_node.node_entries[2].offset);
    EXPECT_EQ(0, empty_node.node_entries[2].record.key);
    EXPECT_STREQ("", empty_node.node_entries[2].record.value);

    EXPECT_EQ(0, empty_node.next_offset);
}