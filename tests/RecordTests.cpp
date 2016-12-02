#include "gtest/gtest.h"
#include "Record.h"

struct RecordBasicTest : public ::testing::Test {
    RecordBasicTest() : record(123, "CB12345") {};

    btree::Record record;
};

TEST_F(RecordBasicTest, GIVENconstructedRecordWHENcheckValueTHENproperValue) {
    EXPECT_EQ(123, record.key);
    EXPECT_STREQ("CB12345", record.value);
}