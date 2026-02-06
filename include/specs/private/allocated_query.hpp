#pragma once

#include <cstdint>

namespace specs {
    struct AllocatedQuery {
        uint8_t c_count;
        uint8_t mutable_c_count;
        uint8_t r_count;
        uint8_t mutable_r_count;
        uint32_t start_index;
    };
}
