module;

#include <string>
#include <span>
#include <vector>
#include <unordered_set>
#include <typeinfo>

#include <print>

export module specs.schedule;

import specs.component;
import specs.component_storage;
import specs.system;

namespace specs {
    struct QueriedComponent {
        std::string name;
        size_t hash_cache;
        bool is_mutable;

        explicit QueriedComponent() {}

        explicit QueriedComponent(std::string name, bool is_mutable)
        : name(std::forward<std::string>(name)), hash_cache(std::hash<std::string>{}(this->name)), is_mutable(is_mutable) {}

        bool operator==(const QueriedComponent& other) const {
            return name == other.name;
        }
    };
}

template<>
struct std::hash<specs::QueriedComponent> {
    size_t operator()(const specs::QueriedComponent& c) const noexcept {
        return c.hash_cache;
    }
};

namespace specs {
    struct ScheduleFrame {
        std::unordered_set<QueriedComponent> used_components;
        size_t end_index;
    };

    export using SystemID = size_t;

    template <typename T>
    struct function_traits : function_traits<decltype(&T::operator())> {};

    template <typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...) const> {
        using args_tuple = std::tuple<Args...>;
    };

    template <typename Func>
    concept SystemFunc = []<typename... Args>(std::tuple<Args...>*) {
        return (ComponentType<std::remove_cvref_t<Args>> && ...);
    }(static_cast<typename function_traits<std::decay_t<Func>>::args_tuple*>(nullptr));

    template <typename T>
    constexpr bool is_const_component_v =
        std::is_lvalue_reference_v<T> &&
        std::is_const_v<std::remove_reference_t<T>>;
    
    template <typename T>
    constexpr bool is_mut_component_v =
        std::is_lvalue_reference_v<T> &&
        !std::is_const_v<std::remove_reference_t<T>>;

    export class Schedule {
    private:
        std::vector<System> systems;

        std::vector<SystemID> ordered;
        std::vector<ScheduleFrame> frames;

        std::vector<System>

        template <SystemFunc Func, typename Tuple, std::size_t... I>
        SystemID register_system_impl(Func&& func, std::index_sequence<I...>) {
            return register_system_expl<Func, std::tuple_element_t<I, Tuple>...>(
                std::forward<Func>(func)
            );
        }

        template <typename T>
        std::string type_name() {
            return typeid(T).name();
        }

        void schedule_system(SystemID id, std::span<QueriedComponent> queried_components) {
            ScheduleFrame* chosen_frame = nullptr;

            for (auto& frame : frames) {
                // Check for query component conflicts in frame
                bool conflict_found = false;

                for (auto& component : queried_components) {
                    auto it = frame.used_components.find(component);
                    if (it != frame.used_components.end() && (it->is_mutable == true || component.is_mutable == true)) {
                        conflict_found = true;
                        break;
                    }
                }
                if (!conflict_found) {
                    chosen_frame = &frame;
                    break;
                }
            }

            if (chosen_frame == nullptr) {
                size_t ending_index = (frames.size() > 0) ? frames.back().end_index : 0;

                chosen_frame = &frames.emplace_back();

                chosen_frame->end_index = ending_index;
            }
            

            for (auto& component : queried_components) {
                chosen_frame->used_components.emplace(std::move(component));
            }

            for (int i = (chosen_frame - frames.data()); i < frames.size(); i++) {
                frames[i].end_index++;
            }
        }

        template <typename Func>
        static void system_thunk(ComponentStorage& cs) {
            Func{}();
        }
    public:
        void update();

        template <SystemFunc Func>
        SystemID register_system(Func&& func) {
            using traits = function_traits<std::decay_t<Func>>;
            using args_tuple = typename traits::args_tuple;

            return register_system_impl<Func, args_tuple>(
                std::forward<Func>(func),
                std::make_index_sequence<std::tuple_size_v<args_tuple>>{}
            );
        }

        template <SystemFunc Func, ComponentType... QueriedComponents>
        SystemID register_system_expl(Func&& func) {
            std::array<QueriedComponent, sizeof...(QueriedComponents)> queried_components;

            [&]<std::size_t... I>(std::index_sequence<I...>) {
                ((queried_components[I] = QueriedComponent(
                    type_name<QueriedComponents>(),
                    is_mut_component_v<QueriedComponents>
                )), ...);
            }(std::make_index_sequence<sizeof...(QueriedComponents)>{});

            SystemID id = systems.size();
            systems.emplace_back(System {
                .func = system_thunk<Func>,
                .disabled = false,
            });

            schedule_system(id, queried_components);

            return id;
        }
        
        void run() {
            int next_index = 0;
            for (auto& frame : frames) {
                for (int i = next_index; i < frame.end_index; i++) {
                    std::println("sdg");
                    ComponentStorage* cs = nullptr;
                    systems[i].func(*cs);
                }
                next_index = frame.end_index + 1;
            }
        }
    };
};
