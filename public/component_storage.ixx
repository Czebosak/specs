module;

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <string>
#include <span>
#include <optional>
#include <any>
#include <utility>
#include <variant>
#include <cassert>

export module specs.component_storage;

import specs.sparse_set;
import specs.entity;
import specs.component;

namespace specs {
    export using ComponentNumericID = size_t;

    export using ResourceNumericID = size_t;

    using RegisteredTypeID = std::variant<ComponentNumericID, ResourceNumericID>;

    export class ComponentStorage {
    private:
        std::vector<std::any> resources;
        std::vector<utils::SparseSet<>> component_data;
        std::unordered_map<size_t, utils::SparseSet<EntityID>> groups;
        std::unordered_map<std::string, ComponentNumericID> type_registry;
    public:
        static size_t get_group_hash(std::span<std::string> components) {
            std::hash<std::string> hasher;
            size_t h = 0;

            for (auto& c : components) {
                c = "s";
                h ^= hasher(c);
            }

            return h;
        }

        std::optional<std::span<EntityID>> get_members_of_group(size_t hash) {
            auto it = groups.find(hash);
            if (it != groups.end()) {
                return std::make_optional(it->second.get_dense_data());
            } else {
                return std::nullopt;
            }
        }

        void add_to_group(size_t hash, EntityID id) {
            auto it = groups.find(hash);
            if (it != groups.end()) {
                it->second.emplace(id, id);
            }
        }

        void add_group(std::span<std::string> components) {
            groups.emplace(get_group_hash(components), utils::SparseSet<EntityID>{});
        }

        template <ComponentType T>
        ComponentNumericID register_component() {
            ComponentNumericID id = component_data.size();
            component_data.emplace_back();
            type_registry.emplace(typeid(T).name(), id);
        }

        template <ComponentType T, typename... Args>
        void emplace_component(EntityID id, Args&&... args) {
            auto it = type_registry.find(typeid(T).name());
            if (it == type_registry.end()) return;

            auto& component_set = component_data[it->second];
            component_set.emplace<T>(id, std::forward(args)...);
        }
        
        template <ComponentType T>
        void erase_component(EntityID id) {
            auto it = type_registry.find(typeid(T).name());
            if (it == type_registry.end()) return;

            component_data[it->second].erase<T>(id);
        }
    };
}
