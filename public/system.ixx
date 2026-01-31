module;

#include <cstdint>

export module specs.system;

import specs.component_storage;

namespace specs {
    export struct System {
        void (*func)(ComponentStorage&);
        uint32_t query_list_index;
        uint16_t query_count;
        bool disabled;
    };
}
