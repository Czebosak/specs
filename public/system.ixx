module;

#include <cstdint>
#include <span>

#include <allocated_query.hpp>

export module specs.system;

import specs.component_storage;
import specs.component;

namespace specs {
    export struct System {
        void (*func)(ComponentStorage&, std::span<AllocatedQuery>, std::span<ComponentID>);
        uint32_t query_list_index;
        uint16_t query_count;
        bool disabled;
    };
}
