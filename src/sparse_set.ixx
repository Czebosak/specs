module;

#include <vector>

export module specs.utils.sparse_set;

import specs.entity;

export template <typename T>
class SparseSet {
private:
    #ifndef SPECS_PAGE_SIZE
    #define SPECS_PAGE_SIZE 1024 // default
    #endif
    static constexpr size_t PAGE_SIZE = SPECS_PAGE_SIZE;
    static constexpr size_t NULL_INDEX = -1;

    struct DenseEntry {
        size_t sparse_index;
        T data;
    };

    std::vector<DenseEntry> dense;
    std::vector<std::vector<size_t>> sparse;

    inline size_t get_page_index(EntityID id) {
        return id / PAGE_SIZE;
    }

    inline size_t get_index_in_page(EntityID id) {
        return id % PAGE_SIZE;
    }
public:
    explicit SparseSet();

    template <typename... Args>
    T& emplace(EntityID id, Args&&... args) {
        size_t dense_i = dense.size();
        T& reference = dense.emplace_back(id, T(std::forward<Args>(args)...));

        size_t page_i = get_page_index(id);
        size_t i = get_index_in_page(id);

        if (page_i >= sparse.size()) {
            sparse.resize(page_i + 1);
            sparse[page_i].resize(i + 1, NULL_INDEX);
        }

        sparse[page_i][i] = dense_i;

        return reference;
    }

    T& get(EntityID id) {
        return dense[sparse[get_page_index(id)][get_index_in_page(id)]].data;
    }

    void erase(EntityID id) {
        size_t page_i = get_page_index(id);
        size_t i = get_index_in_page(id);
        size_t dense_i = sparse[page_i][i];

        DenseEntry last = dense.back().sparse_index;
        std::swap(dense[dense_i], dense.back());

        sparse[page_i][i] = NULL_INDEX;
        sparse[get_page_index(last)][get_index_in_page(last)] = dense_i;
        
        dense.pop_back();
    }
};
