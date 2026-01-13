module;

module specs.world;

specs::World::World() : next_id(0) {}

specs::EntityHandle specs::World::create_entity() {
    if (!recycled_ids.empty()) {
        RecycledEntityID& recycled = recycled_ids.back();
        recycled_ids.pop_back();
        return reinterpret_cast<EntityHandle&>(recycled);
    } else {
        return { .id = next_id++, .generation = 0 };
    }
}
