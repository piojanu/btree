#pragma once

namespace btree {

// Container methods' return/exceptions codes (success, errors...)
enum : int {
    SUCCESS = 0,
    RECORD_NOT_FOUND = -1,
    SOMETHING_WENT_WRONG = -2,
    NOT_IMPLEMENTED = -1000
};

}