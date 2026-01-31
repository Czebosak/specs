module;

#include <vector>

export module specs.commands;

import specs.world;
import specs.entity;
import specs.component_storage;

namespace specs {
    struct Command {
        virtual ~Command();

        virtual void apply(ComponentStorage& world) = 0;
    };

    template <typename T>
    struct InsertComponentCommand : Command {
        EntityID entity;
        T component;
        InsertComponentCommand(EntityID e, T c) : entity(e), component(std::move(c)) {}

        void apply(ComponentStorage& cs) override {
            cs.push_component(entity, component);
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
