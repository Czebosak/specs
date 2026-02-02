module;

#include <cstddef>
#include <cstdint>

export module specs.entity;

namespace specs {
    export using EntityID = uint32_t;
    export using Generation = uint32_t;

    export struct EntityHandle {
        EntityID id;
        Generation generation;

        //EntityHandle& add_component();

        //void destroy();
    };
}
