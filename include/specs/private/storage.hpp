#pragma once

#include <vector>
#include <span>
#include <any>
#include <utility>
#include <cstdint>
#include <cassert>
#include <limits>

#include <ankerl/unordered_dense.h>
#include <boost/container/small_vector.hpp>
#include <ctti/nameof.hpp>

#include <specs/entity.hpp>
#include <specs/component.hpp>
#include <specs/archetype.hpp>

struct Velocity {
    float value[3];
};

struct Position {
    float value[3];
};

namespace specs {
    class Storage {
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

        ComponentID combine_hash(std::span<ComponentID> values);

        // Returns archetype index
        uint32_t create_new_archetype(std::span<ComponentID> components);

        void move_component_data(EntityID id, Archetype* new_archetype);
    public:
        boost::container::small_vector<std::span<uint8_t>, 8> match_archetypes(std::span<ComponentID> queried_components);

        Storage() : next_id(0) {
            EntityHandle handle = spawn_entity();
            insert_component(handle.id, Position { { 10.0f } });
            insert_component(handle.id, Velocity { { 0.0f } });
        }

        EntityHandle spawn_entity();
        void despawn_entity(EntityID id);

        template <ComponentType T>
        void insert_component(EntityID id, T&& c) {
            Archetype* old_archetype = nullptr;
            if (entity_locations[id].archetype != NO_ARCHETYPE) {
                old_archetype = &archetypes[entity_locations[id].archetype];
            };

            ComponentID hash = ankerl::unordered_dense::hash<std::string_view>{}(std::string_view(ctti::nameof<std::remove_cvref_t<T>>()));

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
    };
}
