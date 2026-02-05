module;

#include <cstdint>
#include <span>

#include <allocated_query.hpp>

export module specs.system;

import specs.storage;
import specs.component;

namespace specs {
    export struct System {
        using SystemFunc = void(*)(Storage&, std::span<AllocatedQuery>, std::span<ComponentID>);

        SystemFunc func;
        uint32_t query_list_index;
        uint16_t query_count;
        bool disabled;
    };
}
