module;

#include <string>

export module specs.component;

namespace specs {
    export using ComponentID = std::string;

    export template <typename T>
    concept ComponentType = std::is_class_v<std::remove_cvref_t<T>> || std::is_enum_v<std::remove_cvref_t<T>>;
}