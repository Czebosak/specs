
module;

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string_view>

export module specs.world;

import specs.entity;
import specs.component;
import specs.schedule;

namespace specs {
    export enum class ComponentStorageType {
        SparseSet,
    };
    
    export class World {
    private:
        struct RecycledEntityID {
            EntityID id;
            uint32_t generation;
        };

        static_assert(sizeof(RecycledEntityID) == sizeof(EntityHandle));

        std::vector<RecycledEntityID> recycled_ids;
        size_t next_id;
        Schedule schedule;
    public:
        explicit World();

        // Registers a new component
        // When adding a component with a templated function
        // to an entity that hasn't been registered it will
        // be registered for you
        template <ComponentType T>
        void register_component(ComponentStorageType storage_type = ComponentStorageType::SparseSet) {
        }

        // Registers a new component with an explicit name
        // Adding this component to entities will require
        // you to also explicitly pass the name
        template <typename T>
        void register_component(std::string_view name, ComponentStorageType storage_type = ComponentStorageType::SparseSet) {}

        void register_component(std::string_view name, size_t size, size_t alignment, ComponentStorageType storage_type = ComponentStorageType::SparseSet) {}

        // Creates new entity returning the handle
        // will recycle old IDs of deleted entities
        EntityHandle create_entity();
    };
}
