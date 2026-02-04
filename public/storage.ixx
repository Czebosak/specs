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
#include <cstdint>
#include <cassert>
#include <print>
#include <limits>

#include <ankerl/unordered_dense.h>
#include <boost/container/small_vector.hpp>
#include <ctti/nameof.hpp>

export module specs.storage;

import specs.sparse_set;
import specs.entity;
import specs.component;

import :archetype;

export struct Velocity {
    float value[3];
};

export struct Position {
    float value[3];
};

namespace specs {
    export class Storage {
    private:
        std::vector<EntityID> recycled_ids;
        std::vector<Generation> generations;
        EntityID next_id;

        std::vector<std::any> resources;
        /* std::vector<utils::SparseSet<>> component_data;
        std::unordered_map<size_t, utils::SparseSet<EntityID>> groups; */
        /* std::unordered_map<std::string, ComponentNumericID> type_registry; */
        std::vector<Archetype> archetypes;

        std::vector<EntityLocation> entity_locations;

        ankerl::unordered_dense::map<ComponentID, boost::container::small_vector<uint32_t, 6>> component_to_archetype;
        
        ankerl::unordered_dense::map<ComponentID, uint32_t> component_set_to_archetype;

        struct ComponentInfo {
            uint32_t size;
            void(*destructor)(void*);
        };

        ankerl::unordered_dense::map<ComponentID, ComponentInfo> component_infos;

        static constexpr uint32_t NO_ARCHETYPE = std::numeric_limits<uint32_t>::max();

        ComponentID combine_hash(std::span<ComponentID> values) {
            ComponentID sum = 0;

            for (uint32_t v : values) {
                sum += v;
            }

            return ankerl::unordered_dense::hash<ComponentID>{}(sum);
        }

        // Returns archetype index
        uint32_t create_new_archetype(std::span<ComponentID> components) {
            std::vector<uint32_t> type_sizes(components.size());
            std::vector<void(*)(void*)> destructors(components.size());

            for (int i = 0; i < components.size(); i++) {
                auto it = component_infos.find(components[i]);
                ComponentInfo& info = it->second;
                type_sizes[i] = info.size;
                destructors[i] = info.destructor;
            }

            uint32_t archetype_i = archetypes.size();
            archetypes.emplace_back(type_sizes, components, destructors);
            for (ComponentID component : components) {
                auto it = component_to_archetype.find(component);
                if (it != component_to_archetype.end()) {
                    it->second.emplace_back(archetype_i);
                } else {
                    component_to_archetype.emplace(component, boost::container::small_vector<uint32_t, 6>{ archetype_i });
                }
            }

            ComponentID combined = combine_hash(components);
            component_set_to_archetype.emplace(combined, archetype_i);

            return archetype_i;
        }

        void move_component_data(EntityID id, Archetype* new_archetype) {
            EntityLocation loc = entity_locations[id];

            Archetype& a = archetypes[loc.archetype];

            for (const auto& [k, v] : a.components) {
                auto it = a.components.find(k);
                if (it != a.components.end()) {
                    Archetype::Column& col = a.columns[it->second];
                    void* data = &col.data[loc.row * col.type_size];
                    new_archetype->push(v, data);
                }
            }

            a.erase(loc.row, entity_locations);
        }
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

        Storage() : next_id(0) {
            EntityHandle handle = spawn_entity();
            insert_component(handle.id, Position { { 10.0f } });
            insert_component(handle.id, Velocity { { 0.0f } });
        }

        EntityHandle spawn_entity() {
            EntityHandle handle;

            if (!recycled_ids.empty()) {
                EntityID id = recycled_ids.back();
                recycled_ids.pop_back();
                handle = { .id = id, .generation = generations[id] };
                entity_locations[handle.id] = { NO_ARCHETYPE, 0 };
            } else {
                handle = { .id = next_id++, .generation = 0 };
                entity_locations.emplace_back(NO_ARCHETYPE, 0);
                if (generations.size() + 1 < handle.id) {}
                generations.emplace_back(0);
            }

            return handle;
        }

        void despawn_entity(EntityID id) {
            if (id + 1 == entity_locations.size())
                entity_locations.pop_back();

            recycled_ids.emplace_back(id);
            generations[id]++;
        }

        template <ComponentType T>
        void insert_component(EntityID id, T&& c) {
            Archetype* old_archetype = nullptr;
            if (entity_locations[id].archetype != NO_ARCHETYPE) {
                old_archetype = &archetypes[entity_locations[id].archetype];
            };

            ComponentID hash = ankerl::unordered_dense::hash<std::string_view>{}(std::string_view(ctti::nameof<T>()));

            if (!component_infos.contains(hash)) {
                component_infos.emplace(hash, ComponentInfo { sizeof(T), nullptr });
            }

            ComponentID set_hash = hash;
            if (old_archetype != nullptr) {
                for (const auto& [c, _] : old_archetype->components) {
                    set_hash += c;
                }
                set_hash = ankerl::unordered_dense::hash<ComponentID>{}(hash);
            }

            uint32_t new_archetype_idx;

            auto it = component_set_to_archetype.find(set_hash);
            if (it != component_set_to_archetype.end()) {
                new_archetype_idx = it->second;
            } else {
                boost::container::small_vector<ComponentID, 6> components;
                if (old_archetype != nullptr) {
                    for (const auto& [c, _] : old_archetype->components) {
                        components.emplace_back(c);
                    }
                }
                components.emplace_back(hash);

                new_archetype_idx = create_new_archetype(components);
            }

            Archetype& new_archetype = archetypes[new_archetype_idx];

            uint32_t row = new_archetype.entities.size();

            if (old_archetype != nullptr) {
                move_component_data(id, &new_archetype);
            }

            new_archetype.push_entity(id);
            new_archetype.push(hash, static_cast<void*>(&c));

            entity_locations[id].archetype = new_archetype_idx;
            entity_locations[id].row = row;
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
