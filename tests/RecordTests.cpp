#include "gtest/gtest.h"
#include "Record.h"

struct RecordBasicTest : public ::testing::Test {
    RecordBasicTest() : empty_record() {
        inner_record.key = 123;
        strcpy(inner_record.value, "CB12345");
    }

    btree::Record inner_record, leaf_record, empty_record;
};

TEST_F(RecordBasicTest, GIVENconstructedInnerRecordWHENcheckValueTHENproperValue) {
    EXPECT_EQ(123, inner_record.key);
}


TEST_F(RecordBasicTest, GIVENconstructedLeafRecordWHENcheckValueTHENproperValue) {
    EXPECT_STREQ("CB12345", leaf_record.value);
}

TEST_F(RecordBasicTest, GIVENemptyRecordWHENcheckValueTHENproperValue) {
    EXPECT_EQ(0, empty_record.key);
    EXPECT_STREQ("", empty_record.value);
}