module;

#include <algorithm>
#include <iterator>
#include <string>
#include <string_view>
#include <typeinfo>
#include <cstdint>

#include <ctti/nameof.hpp>
#include <boost/container/small_vector.hpp>
#include <ankerl/unordered_dense.h>

#include <allocated_query.hpp>

export module specs.schedule;

import specs.component;
import specs.component_storage;
import specs.system;
import specs.query;

namespace specs {
    struct ComponentData {
        bool is_mutable;
    };

    struct ScheduleFrame {
        ankerl::unordered_dense::map<size_t, ComponentData> used_components;
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

        std::vector<size_t> query_data;
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

        template <std::size_t N, typename T>
        static consteval std::array<T, N> to_array(const std::vector<T>& v) {
            std::array<T, N> a{};
            for (std::size_t i = 0; i < N; ++i)
                a[i] = v[i];
            return a;
        }

        template <typename... QueriedComponents>
        static consteval auto parse_query() {
            constexpr auto counts = count_categories();

            constexpr ParsedQuery parsed = [&]() {
                std::vector<std::string_view> immutable_components;
                std::vector<std::string_view> mutable_components;

                immutable_components.reserve(std::get<0>(counts));
                mutable_components.reserve(std::get<1>(counts));

                ([&]<typename T>() {
                    std::string_view name = std::string_view(ctti::nameof<T>().begin(), ctti::nameof<T>().end());

                    if constexpr (is_const_ref_v<T>) {
                        immutable_components.emplace_back(name);
                    } else if constexpr (is_mut_ref_v<T>) {
                        mutable_components.emplace_back(name);
                    }
                }.template operator()<QueriedComponents>(), ...);

                return ParsedQuery<std::get<0>(counts), std::get<1>(counts), std::get<2>(counts), std::get<3>(counts)> {
                    to_array<std::get<0>(counts)>(immutable_components),
                    to_array<std::get<1>(counts)>(mutable_components),
                };
            }();

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

        template <typename Func>
        inline void for_each_component_in_system_query(std::span<uint32_t> query_indices, Func&& func) {
            for (uint32_t query_index : query_indices) {
                AllocatedQuery q = allocated_queries[query_index];

                for (int i = 0; i < q.c_count + q.mutable_c_count + q.r_count + q.mutable_r_count; i++) {
                    ComponentID component_id = query_data[q.start_index + i];
                    bool is_mutable = i >= q.c_count;

                    bool should_break = func(component_id, is_mutable);
                    if (should_break) break;
                }
            }
        }

        void schedule_system(SystemID id, std::span<uint32_t> query_indices) {
            ScheduleFrame* chosen_frame = nullptr;

            for (auto& frame : frames) {
                // Check for query component conflicts in frame
                bool conflict_found = false;

                for_each_component_in_system_query(query_indices, [&](ComponentID component_id, bool is_mutable) {
                    auto it = frame.used_components.find(component_id);
                    if (it != frame.used_components.end() && (it->second.is_mutable || is_mutable)) {
                        conflict_found = true;
                        return true;
                    }

                    return false;
                });

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
            
            for_each_component_in_system_query(query_indices, [chosen_frame](ComponentID component_id, bool is_mutable) {
                chosen_frame->used_components.emplace(component_id, is_mutable);
                return true;
            });

            for (int i = (chosen_frame - frames.data()); i < frames.size(); i++) {
                frames[i].end_index++;
            }
        }

        template <typename Func, typename... Parameters>
        static void system_thunk(ComponentStorage& cs, std::span<AllocatedQuery> queries, std::span<ComponentID> query_data) {
            std::tuple<Parameters...> parameter_tuple;

            int query_idx = 0;

            [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                ([&]<std::size_t I>() {
                    using Parameter = std::tuple_element_t<I, std::tuple<Parameters...>>;

                    if constexpr (SpecializationOf<Parameter, Query>) {
                        const AllocatedQuery& q = queries[query_idx];
                        std::span components{&query_data[q.start_index], static_cast<size_t>(q.c_count + q.mutable_c_count)};

                        auto matched = cs.match_archetypes(components);

                        std::get<I>(parameter_tuple) = Parameter(matched);
                    }
                }.template operator()<Is>(), ...);
            }(std::index_sequence_for<Parameters...>{});

            std::apply(Func{}, parameter_tuple);
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
                    constexpr auto parsed = QueryParsingHelper<Parameter>{}();

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

                    ankerl::unordered_dense::hash<std::string_view> hasher;

                    auto append_hashed = [&](const auto& src) {
                        std::transform(src.begin(), src.end(),
                            std::back_inserter(query_data),
                            [&](const auto& v) { return hasher(v); }
                        );
                    };

                    append_hashed(parsed.immutable_components);
                    append_hashed(parsed.mutable_components);
                    append_hashed(parsed.immutable_resources);
                    append_hashed(parsed.mutable_resources);
                }
            }.template operator()<Parameters>(), ...);

            SystemID id = systems.size();
            systems.emplace_back(System {
                .func = system_thunk<Func, Parameters...>,
                .disabled = false,
            });

            schedule_system(id, query_indices);

            return id;
        }

        void run(ComponentStorage& cs) {
            int next_index = 0;
            for (auto& frame : frames) {
                for (int i = next_index; i < frame.end_index; i++) {
                    systems[i].func(cs, std::span<AllocatedQuery>{allocated_queries}, query_data);
                }
                next_index = frame.end_index + 1;
            }
        }
    };
};
