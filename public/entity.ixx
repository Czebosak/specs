module;

#include <cstddef>
#include <cstdint>

export module specs.entity;

namespace specs {
    export using EntityID = size_t;

    export struct EntityHandle {
        EntityID id;
        uint32_t generation;

        //EntityHandle& add_component();

        //void destroy();
    };
}
