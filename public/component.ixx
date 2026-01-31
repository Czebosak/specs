module;

#include <cstdint>
#include <type_traits>

export module specs.component;

namespace specs {
    export using ComponentID = uint32_t;

    export template <typename T>
    concept ComponentType = std::is_class_v<T>
                         || std::is_enum_v<T>;
}
