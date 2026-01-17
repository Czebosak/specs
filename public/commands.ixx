module;

#include <vector>

export module specs.commands;

import specs.world;
import specs.entity;

namespace specs {
    struct Command {
        virtual ~Command();

        virtual void apply(World& world) = 0;
    };

    template <typename T>
    struct InsertComponentCommand : Command {
        int entity;
        T component;
        InsertComponentCommand(int e, T c) : entity(e), component(std::move(c)) {}
        void apply(World& world) override {
            world.insertComponent<T>(entity, std::move(component));
        }
    };

    export class EntityCommands;

    export class Commands {
    private:
        specs::World* world;
        std::vector<Command> command_queue;
    public:
        EntityCommands spawn();
    };

    export class EntityCommands {
    private:
        EntityHandle handle;
    };
}
