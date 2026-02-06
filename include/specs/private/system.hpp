#pragma once

#include <cstdint>
#include <span>

#include <specs/private/allocated_query.hpp>

#include <specs/component.hpp>

namespace specs {
    class Storage;

    struct System {
        using SystemFunc = void(*)(Storage&, std::span<AllocatedQuery>, std::span<ComponentID>);

        SystemFunc func;
        uint32_t query_list_index;
        uint16_t query_count;
        bool disabled;
    };
}
