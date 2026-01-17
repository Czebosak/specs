module;

#include <string>
#include <type_traits>

export module specs.component;

#include "macros.hpp"

namespace specs {
    export using ComponentID = std::string;
    
    export template <typename T>
    struct IsResource : std::false_type {};

    export template <typename T>
    struct IsComponent : std::false_type {};

    export template <typename T>
    concept ComponentType = IsComponent<T>::value;
}