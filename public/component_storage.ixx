module;

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <string>
#include <span>
#include <optional>

export module specs.component_storage;

import specs.sparse_set;
import specs.entity;

namespace specs {
    export using ComponentNumericID = size_t;

    struct ComponentDefinition {
        utils::SparseSet<> data;
        size_t component_size;
    };

    export class ComponentStorage {
    private:
        std::vector<ComponentDefinition> component_data;
        std::unordered_map<size_t, utils::SparseSet<EntityID>> groups;
        std::unordered_map<std::string, ComponentNumericID> component_to_id;
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
            groups.find(hash);
        }
    };
}
