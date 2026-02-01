module;

#include <cstddef>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <span>
#include <optional>
#include <any>
#include <utility>
#include <variant>
#include <cassert>

#include <ankerl/unordered_dense.h>
#include <boost/container/small_vector.hpp>
#include <ctti/nameof.hpp>

export module specs.component_storage;

import specs.sparse_set;
import specs.entity;
import specs.component;

import :archetype;

export struct Velocity {
    float value[3];
};

namespace specs {
    export class ComponentStorage {
    private:
        std::vector<std::any> resources;
        /* std::vector<utils::SparseSet<>> component_data;
        std::unordered_map<size_t, utils::SparseSet<EntityID>> groups; */
        /* std::unordered_map<std::string, ComponentNumericID> type_registry; */
        std::vector<Archetype> archetypes;

        std::vector<EntityLocation> entity_locations;

        ankerl::unordered_dense::map<ComponentID, boost::container::small_vector<uint32_t, 6>> component_to_archetype;
    public:
        auto match_archetypes(std::span<ComponentID> queried_components) {
            boost::container::small_vector<Archetype*, 32> matched;
            for (ComponentID queried_component : queried_components) {
                auto it = component_to_archetype.find(queried_component);
                if (it == component_to_archetype.end()) {
                    matched.clear();
                    break;
                }
                std::span<uint32_t> archetype_indices = it->second;

                if (matched.size() == 0) {
                    for (uint32_t i : archetype_indices) {
                        Archetype& a = archetypes[i];

                        matched.emplace_back(&a);
                    }
                } else {
                    boost::container::small_vector<Archetype*, 32> intersection;

                    for (uint32_t i : archetype_indices) {
                        Archetype& a = archetypes[i];

                        if (std::find(matched.begin(), matched.end(), &a) != matched.end()) {
                            intersection.emplace_back(&a);
                        }
                    }
                }
            }

            boost::container::small_vector<std::span<uint8_t>, 8> result;
            for (Archetype* a : matched) {
                for (ComponentID queried_component : queried_components) {
                    result.emplace_back(a->get_column_data(queried_component));
                }
            }
            return result;
        }

        ComponentStorage() {
            uint32_t i = archetypes.size();
            size_t type_sizes[] = { sizeof(Velocity) };
            ComponentID component_id[] = { ankerl::unordered_dense::hash<std::string_view>{}(std::string_view(ctti::nameof<Velocity>())) };
            archetypes.emplace_back(type_sizes, component_id);
            component_to_archetype.emplace(component_id[0], boost::container::small_vector<uint32_t, 6>{ i });
            Velocity v = { {0.0f, 0.0f, 0.0f} };
            archetypes[i].push(component_id[0], &v);
        }

        /* template <ComponentType T>
        void push_component(EntityID id, T&& e) {
            auto it = type_registry.find(typeid(T).name());
            if (it == type_registry.end()) return;

            auto& component_set = component_data[it->second];
            component_set.push<T>(id, std::forward<T>(e));
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
        } */
    };
}
