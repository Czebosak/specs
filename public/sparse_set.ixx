module;

#include <vector>
#include <cstddef>
#include <cstdint>
#include <cassert>

export module specs.sparse_set;

import specs.entity;

namespace specs::utils {
    export class SparseSet {
    private:
        #ifndef SPECS_PAGE_SIZE
        #define SPECS_PAGE_SIZE 1024 // default
        #endif
        static constexpr size_t PAGE_SIZE = SPECS_PAGE_SIZE;
        static constexpr size_t NULL_INDEX = -1;

        std::vector<uint8_t> dense;
        std::vector<EntityID> dense_to_id;
        std::vector<std::vector<size_t>> sparse;

        //void destructor();

        inline size_t get_page_index(EntityID id) {
            return id / PAGE_SIZE;
        }

        inline size_t get_index_in_page(EntityID id) {
            return id % PAGE_SIZE;
        }
    public:
        template <typename T, typename... Args>
        T& emplace(EntityID id, Args&&... args) {
            assert(reinterpret_cast<size_t>(dense.data()) % alignof(T) == 0);

            size_t offset = dense.size();

            if (offset >= dense.capacity() - sizeof(T)) {
                dense.reserve(dense.capacity() * 2);
            }

            dense.resize(offset + sizeof(T));

            T* ptr = new (&dense[offset]) T(std::forward<Args>(args)...); 
            dense_to_id.emplace_back(id);

            size_t page_i = get_page_index(id);
            size_t i = get_index_in_page(id);

            if (page_i >= sparse.size()) {
                sparse.resize(page_i + 1);
            }

            if (i >= sparse[page_i].capacity()) {
                sparse[page_i].resize(i + 1, NULL_INDEX);
            }

            sparse[page_i][i] = offset;

            return *ptr;
        }

        template <typename T>
        T& get(EntityID id) {
            size_t offset = sparse[get_page_index(id)][get_index_in_page(id)] * sizeof(T);
            return *reinterpret_cast<T*>(dense[offset]);
        }

        template <typename T>
        void erase(EntityID id) {
            size_t page_i = get_page_index(id);
            size_t i = get_index_in_page(id);
            size_t offset = sparse[page_i][i];

            EntityID last_id = dense_to_id.back();
            std::swap_ranges(
                dense.begin() + offset,
                dense.begin() + offset + sizeof(T),
                dense.end() - sizeof(T)
            );

            std::swap(dense_to_id[offset / sizeof(T)], dense_to_id.back());

            sparse[page_i][i] = NULL_INDEX;
            sparse[get_page_index(last_id)][get_index_in_page(last_id)] = offset;
            
            dense.resize(dense.size() - sizeof(T));
            dense_to_id.pop_back();
        }
    };
}
