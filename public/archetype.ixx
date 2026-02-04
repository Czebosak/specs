module;

#include <cstdint>
#include <vector>
#include <span>
#include <algorithm>

#include <ankerl/unordered_dense.h>
#include <boost/container/small_vector.hpp>

export module specs.storage:archetype;

import specs.entity;
import specs.component;

namespace specs {
    export class Archetype;

    export struct EntityLocation {
        uint32_t archetype;
        uint32_t row;
    };

    export class Storage;

    export class Archetype {
    private:
        using Destructor = void(*)(void*);

        struct Column {
            std::vector<uint8_t> data;
            uint32_t type_size;
            Destructor destructor;
        };

        std::vector<EntityID> entities;
        ankerl::unordered_dense::map<ComponentID, uint32_t> components;
        boost::container::small_vector<Column, 4> columns;

        friend specs::Storage;
    public:
        Archetype(std::span<uint32_t> type_sizes, std::span<ComponentID> component_ids, std::span<Destructor> destructors) {
            columns.resize(type_sizes.size());
            for (int i = 0; i < type_sizes.size(); i++) {
                columns[i].type_size = type_sizes[i];
                columns[i].destructor = destructors[i];
                components.emplace(component_ids[i], i);
            }
        }

        inline bool is_empty() {
            return entities.empty();
        }

        void push_entity(EntityID id) {
            entities.emplace_back(id);
        }
        
        void push(uint32_t column_index, void* data) {
            Column& column = columns[column_index];

            if (column.data.capacity() == column.data.size()) {
                column.data.reserve(column.data.size() * 2);
            }

            size_t old_size = column.data.size();

            column.data.resize(column.data.size() + column.type_size);

            memcpy(column.data.data() + old_size, data, column.type_size);
        }

        void push(ComponentID component_id, void* data) {
            auto it = components.find(component_id);
            push(it->second, data);
        }

        void erase(uint32_t row, std::span<EntityLocation> entity_locations) {
            bool is_last = row + 1 == entities.size();
            if (!is_last) {
                EntityID last_id = entities.back();
                entity_locations[last_id].row = row;
            }

            std::swap(entities[row], entities.back());
            entities.resize(entities.size() - 1);

            for (Column& column : columns) {
                if (!is_last) {
                    size_t last_row = entities.size() - 1;
                    std::swap_ranges(
                        column.data.data() + row * column.type_size,
                        column.data.data() + row * column.type_size + column.type_size,
                        column.data.data() + last_row * column.type_size
                    );
                }

                column.data.resize(column.data.size() - column.type_size);
            }
        }

        std::span<uint8_t> get_column_data(ComponentID component_id) {
            auto it = components.find(component_id);
            return columns[it->second].data;
        }
    };
}
