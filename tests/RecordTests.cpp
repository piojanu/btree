#include "gtest/gtest.h"
#include "Record.h"

struct RecordBasicTest : public ::testing::Test {
    RecordBasicTest() : record(123, "CB12345"), empty_record() {};

    btree::Record record, empty_record;
};

TEST_F(RecordBasicTest, GIVENconstructedRecordWHENcheckValueTHENproperValue) {
    EXPECT_EQ(123, record.key);
    EXPECT_STREQ("CB12345", record.value);
}

TEST_F(RecordBasicTest, GIVENemptyRecordWHENcheckValueTHENproperValue) {
    EXPECT_EQ(0, empty_record.key);
    EXPECT_STREQ("", empty_record.value);
}