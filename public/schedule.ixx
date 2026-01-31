#include <string_view>
module;

#include <string>
#include <typeinfo>
#include <print>
#include <unordered_set>
#include <cstdint>

#include <ctti/nameof.hpp>

#include <boost/container/small_vector.hpp>

export module specs.schedule;

import specs.component;
import specs.component_storage;
import specs.system;
import specs.query;
import :archetype;

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
        std::unordered_set<std::string_view> used_components;
        size_t end_index;
    };

    export using SystemID = size_t;

    template <typename T, template <typename...> class Template>
    struct is_specialization_of : std::false_type {};

    template <template <typename...> class Template, typename... Args>
    struct is_specialization_of<Template<Args...>, Template> : std::true_type {};

    template <typename T, template <typename...> class Template>
    concept SpecializationOf = is_specialization_of<T, Template>::value;

    template <typename T, typename... Ts>
    concept OneOf = (std::same_as<T, Ts> || ...);

    template <typename T>
    concept SystemParameterType = SpecializationOf<T, Query>;

    template <typename T>
    struct function_traits : function_traits<decltype(&T::operator())> {};

    template <typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...) const> {
        using args_tuple = std::tuple<Args...>;
    };

    template <typename Func>
    concept SystemFunc = []<typename... Args>(std::tuple<Args...>*) {
        return (SystemParameterType<std::remove_cvref_t<Args>> && ...);
    }(static_cast<typename function_traits<std::decay_t<Func>>::args_tuple*>(nullptr));

    template <typename T>
    constexpr bool is_const_ref_v =
        std::is_lvalue_reference_v<T> &&
        std::is_const_v<std::remove_reference_t<T>>;
    
    template <typename T>
    constexpr bool is_mut_ref_v =
        std::is_lvalue_reference_v<T> &&
        !std::is_const_v<std::remove_reference_t<T>>;

    export class Schedule {
    private:
        std::vector<System> systems;

        std::vector<SystemID> ordered;
        std::vector<ScheduleFrame> frames;

        struct AllocatedQuery {
            uint8_t c_count;
            uint8_t mutable_c_count;
            uint8_t r_count;
            uint8_t mutable_r_count;
            uint32_t start_index;
        };

        std::vector<std::string_view> query_data;
        std::vector<AllocatedQuery> allocated_queries;

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

        template <size_t IMMUTABLE_COMPONENT_COUNT, size_t MUTABLE_COMPONENT_COUNT, size_t IMMUTABLE_RESOURCE_COUNT, size_t MUTABLE_RESOURCE_COUNT>
        struct ParsedQuery {
            std::array<std::string_view, IMMUTABLE_COMPONENT_COUNT> immutable_components;
            std::array<std::string_view, MUTABLE_COMPONENT_COUNT> mutable_components;
            std::array<std::string_view, IMMUTABLE_RESOURCE_COUNT> immutable_resources;
            std::array<std::string_view, MUTABLE_RESOURCE_COUNT> mutable_resources;
        };

        template <typename... QueriedComponents>
        static consteval auto count_categories() {
            int imm_c = 0, m_c = 0, imm_r = 0, m_r = 0;

            ([&]<typename T>() {
                if constexpr (is_const_ref_v<T>) {
                    imm_c++;
                } else if constexpr (is_mut_ref_v<T>) {
                    m_c++;
                }
            }.template operator()<QueriedComponents>(), ...);

            return std::tuple{imm_c, m_c, imm_r, m_r};
        }

        template <typename... QueriedComponents>
        static consteval auto parse_query() {
            constexpr auto counts = count_categories();
            ParsedQuery<std::get<0>(counts), std::get<1>(counts), std::get<2>(counts), std::get<3>(counts)> parsed;

            int imm_c = 0, m_c = 0;

            ([&]<typename T>() {
                std::string_view name = std::string_view(ctti::nameof<T>().begin(), ctti::nameof<T>().end());

                if constexpr (is_const_ref_v<T>) {
                    parsed.immutable_components[imm_c++] = name;
                } else if constexpr (is_mut_ref_v<T>) {
                    parsed.mutable_components[m_c++] = name;
                }
            }.template operator()<QueriedComponents>(), ...);

            return parsed;
        }

        template <typename QueryType>
        struct QueryParsingHelper;

        template <typename... QueriedComponents>
        struct QueryParsingHelper<Query<QueriedComponents...>> {
            consteval auto operator()() {
                return parse_query<QueriedComponents...>();
            }
        };

        void schedule_system(SystemID id, std::span<uint32_t> query_indices) {
            ScheduleFrame* chosen_frame = nullptr;

            for (auto& frame : frames) {
                // Check for query component conflicts in frame
                bool conflict_found = false;

                for (uint32_t query_index : query_indices) {
                    AllocatedQuery q = allocated_queries[query_index];

                    for (int i = 0; i < q.c_count + q.mutable_c_count + q.r_count + q.mutable_r_count; i++) {
                        std::string_view component_name = query_data[i + q.start_index];

                        auto it = frame.used_components.find(component_name);
                        if (it != frame.used_components.end() && (it->is_mutable == true || component.is_mutable == true)) {
                            conflict_found = true;
                        }
                    }

                    if (conflict_found) break;
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
        static void system_thunk(ComponentStorage& cs, std::span<AllocatedQuery> queries, std::span<std::string_view> query_data) {

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

        template <SystemFunc Func, SystemParameterType... Parameters>
        SystemID register_system_expl(Func&& func) {
            std::vector<uint32_t> query_indices;

            ([&]<typename Parameter>() {
                if constexpr (SpecializationOf<Parameter, Query>) {
                    constexpr auto parsed = QueryParsingHelper<Parameter>();

                    allocated_queries.emplace_back(
                        parsed.immutable_components.size(),
                        parsed.mutable_components.size(),
                        parsed.immutable_resources.size(),
                        parsed.mutable_resources.size(),
                        query_data.size() + 1
                    );

                    size_t total_size = parsed.immutable_components.size() +
                                        parsed.mutable_components.size() +
                                        parsed.immutable_resources.size() +
                                        parsed.mutable_resources.size();
                    
                    query_data.reserve(total_size);

                    query_data.insert(query_data.end(), parsed.immutable_components.begin(), parsed.immutable_components.end());
                    query_data.insert(query_data.end(), parsed.mutable_components.begin(), parsed.mutable_components.end());
                    query_data.insert(query_data.end(), parsed.immutable_resources.begin(), parsed.immutable_resources.end());
                    query_data.insert(query_data.end(), parsed.mutable_resources.begin(), parsed.mutable_resources.end());
                }
            }.template operator()<Parameters>(), ...);

            SystemID id = systems.size();
            systems.emplace_back(System {
                .func = system_thunk<Func>,
                .disabled = false,
            });

            schedule_system(id, query_indices);

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
