module;

#include <cstddef>

export module specs.entity;

export using EntityID = size_t;

export class Entity {
    unsigned int generation;
};
