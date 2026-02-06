#pragma once

#include <cstdint>

namespace specs {
    using EntityID = uint32_t;
    using Generation = uint32_t;

    struct EntityHandle {
        EntityID id;
        Generation generation;
    };
}
