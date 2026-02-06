#pragma once

#include <vector>
#include <memory>

#include <specs/private/storage.hpp>

#include <specs/entity.hpp>
#include <specs/component.hpp>

namespace specs {
    struct Command {
        virtual ~Command();

        virtual void apply(Storage& storage) = 0;
    };

    struct SpawnEntityCommand : Command {
        void apply(Storage& storage) override {}
    };

    template <typename T>
    struct InsertComponentCommand : Command {
        EntityID entity;
        T component;
        InsertComponentCommand(EntityID e, T c) : entity(e), component(std::move(c)) {}

        void apply(Storage& storage) override {
            //cs.push_component(entity, component);
        }
    };

    struct RuntimeCommand : Command {
        void(*apply_ptr)(Storage&);

        void apply(Storage& storage) override {
            apply_ptr(storage);
        }
    };

    using CommandQueue = std::vector<std::unique_ptr<Command>>;

    class EntityCommands;

    class Commands {
    private:
        CommandQueue& command_queue;
    public:
        Commands(CommandQueue& command_queue) : command_queue(command_queue) {}

        void spawn() {
            command_queue.emplace_back(std::make_unique<SpawnEntityCommand>());
        }
    };

    class EntityCommands {
    private:
        CommandQueue& command_queue;
        EntityID entity_id;
    };
}
