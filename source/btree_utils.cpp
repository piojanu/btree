#include "btree_utils.h"

namespace btree {

SInternalInfo g_iinfo;

std::string code_to_string(int err_code) {
    switch (err_code) {
        case SUCCESS:
            return "Sukces";
        case RECORD_NOT_FOUND:
            return "Rekord nie znaleziony";
        case SOMETHING_WENT_WRONG:
            return "Cos poszlo nie tak";
        case INVALID_OFFSET:
            return "Bledny offset";
        case RECORD_EXISTS:
            return "Rekord istnieje";
        case EMPTY_STORAGE:
            return "Kontener pusty";
        case NOT_ENOUGH_SPACE:
            return "Brak miejsca";
        case INVALID_KEY:
            return "Bledny klucz";
    }

    return "Bledny kod bledu";
}

}
