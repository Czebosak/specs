module;

#include <cstdint>
#include <span>
#include <string_view>

#include <allocated_query.hpp>

export module specs.system;

import specs.component_storage;

namespace specs {
    export struct System {
        void (*func)(ComponentStorage&, std::span<AllocatedQuery>, std::span<std::string_view>);
        uint32_t query_list_index;
        uint16_t query_count;
        bool disabled;
    };
}
