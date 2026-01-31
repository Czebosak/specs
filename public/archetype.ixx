module;

#include <tuple>
#include <vector>

export module specs.schedule:archetype;

import specs.entity;

namespace specs {
    export struct ArchetypeBase {
        std::vector<EntityID> entities;
        virtual ~ArchetypeBase() = default;
    };

    export template <typename... ArchetypeComponents>
    struct Archetype : ArchetypeBase {
        std::tuple<std::vector<ArchetypeComponents>...> columns;
    };
}
