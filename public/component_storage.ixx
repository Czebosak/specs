module;

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <string>

export module specs.component_storage;

import specs.sparse_set;

namespace specs {
    export using ComponentNumericID = size_t;

    struct ComponentDefinition {
        utils::SparseSet data;
        size_t component_size;
    };

    export class ComponentStorage {
    private:
        std::vector<ComponentDefinition> component_data;
        std::unordered_map<std::string, ComponentNumericID> component_to_id;
    public:
        
    };
}
