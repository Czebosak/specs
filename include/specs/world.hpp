#pragma once
#include "specs/private/schedule.hpp"
#include <thread>

#include <specs/entity.hpp>
#include <specs/component.hpp>
#include <specs/commands.hpp>
#include <specs/private/storage.hpp>
#include <specs/private/scheduler.hpp>
#include <specs/private/worker_pool.hpp>

namespace specs {
    class Worker;

    enum class ComponentStorageType {
        SparseSet,
        Archetype,
    };
    
    class World {
    private:
        Storage storage;
        Schedule schedule;
        Scheduler scheduler;
        WorkerPool worker_pool;

        CommandQueue command_queue;
    public:
        World(unsigned int worker_count = std::thread::hardware_concurrency());

        ~World();

        // Registers a new component
        // When adding a component with a templated function
        // to an entity that hasn't been registered it will
        // be registered for you
        /* template <ComponentType T>
        ComponentNumericID register_component(ComponentStorageType storage_type = ComponentStorageType::SparseSet) {
            return component_storage.register_component<T>();
        } */

        // Registers a new component with an explicit name
        // Adding this component to entities will require
        // you to also explicitly pass the name
        /* template <typename T>
        void register_component(std::string_view name, ComponentStorageType storage_type = ComponentStorageType::SparseSet) {} */

        /* void register_component(std::string_view name, size_t size, size_t alignment, ComponentStorageType storage_type = ComponentStorageType::SparseSet) {} */

        // Temporary
        Schedule& get_schedule() {
            return schedule;
        }

        template <typename S, SystemFuncType System>
        void add_system(S schedule, System&& system) {
            size_t id = ScheduleLabelRegistry::id<S>();
            auto opt = scheduler.get_schedule(id);
            Schedule* schedule_ptr;
            if (opt) {
                schedule_ptr = opt.value();
            } else {
                schedule_ptr = scheduler.add_schedule(id);
            }
            schedule_ptr->register_system(std::forward(system));
        }

        template <typename S, SystemFuncType... Systems>
        void add_systems(S schedule, Systems&&... systems) {
            size_t id = ScheduleLabelRegistry::id<S>();
            auto opt = scheduler.get_schedule(id);
            Schedule* schedule_ptr;
            if (opt) {
                schedule_ptr = opt.value();
            } else {
                schedule_ptr = scheduler.add_schedule(id);
            }
            (schedule_ptr->register_system(std::forward<Systems>(systems)), ...);
        }

        void run();
    };
}
