#pragma once

#include <cstdint>

namespace btree {

// Container methods' return/exceptions codes (success, errors...)
enum : int {
    SUCCESS = 0,
    RECORD_NOT_FOUND = -1,
    SOMETHING_WENT_WRONG = -2,
    INVALID_OFFSET = -3,
    RECORD_EXISTS = -4,
    EMPTY_STORAGE = -5,
    NOT_IMPLEMENTED = -1000
};

struct SInternalInfo {
    uint64_t reads;
    uint64_t writes;

    void reset() {
        reads = 0;
        writes = 0;
    }
};

extern SInternalInfo g_iinfo;

}