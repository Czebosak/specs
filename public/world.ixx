
module;

#include <cstddef>
#include <cstdint>
#include <vector>

export module specs.world;

import specs.entity;
import specs.component;
import specs.storage;
import specs.schedule;

namespace specs {
    export enum class ComponentStorageType {
        SparseSet,
    };
    
    export class World {
    private:
        Storage storage;
        Schedule schedule;
    public:
        // Registers a new component
        // When adding a component with a templated function
        // to an entity that hasn't been registered it will
        // be registered for you
        /* template <ComponentType T>
        ComponentNumericID register_component(ComponentStorageType storage_type = ComponentStorageType::SparseSet) {
            return component_storage.register_component<T>();
        } */

        // Registers a new component with an explicit name
        // Adding this component to entities will require
        // you to also explicitly pass the name
        /* template <typename T>
        void register_component(std::string_view name, ComponentStorageType storage_type = ComponentStorageType::SparseSet) {} */

        /* void register_component(std::string_view name, size_t size, size_t alignment, ComponentStorageType storage_type = ComponentStorageType::SparseSet) {} */

        // Temporary
        Schedule& get_schedule() {
            return schedule;
        }

        void run() {
            schedule.run(storage);
        }
    };
}
