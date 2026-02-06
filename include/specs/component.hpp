#pragma once

#include <cstddef>
#include <type_traits>

namespace specs {
    using ComponentID = size_t;

    template <typename T>
    concept ComponentType = std::is_class_v<T>
                         || std::is_enum_v<T>;
}
