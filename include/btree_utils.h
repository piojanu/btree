#pragma once

#include <cstdint>
#include <string>

namespace btree {

// Container methods' return/exceptions codes (success, errors...)
enum ErrorCode : int {
    SUCCESS = 0,
    RECORD_NOT_FOUND = -1,
    SOMETHING_WENT_WRONG = -2,
    INVALID_OFFSET = -3,
    RECORD_EXISTS = -4,
    EMPTY_STORAGE = -5,
    NOT_ENOUGH_SPACE = -6,
    INVALID_KEY = -7,
};

std::string code_to_string(int err_code);

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