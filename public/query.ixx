module;

#include <tuple>
#include <span>
#include <type_traits>
#include <cstdint>
#include <string_view>
#include <vector>

export module specs.query;

import specs.component;

namespace specs {
    class Schedule;

    export template <typename T>
    concept QueriedComponentType = std::is_lvalue_reference_v<T> ||
                                   ComponentType<std::remove_cvref_t<T>>;
    
    template <typename T>
    constexpr bool is_const_ref_v =
        std::is_lvalue_reference_v<T> &&
        std::is_const_v<std::remove_reference_t<T>>;
    
    template <typename T>
    constexpr bool is_mut_ref_v =
        std::is_lvalue_reference_v<T> &&
        !std::is_const_v<std::remove_reference_t<T>>;

    /* template <typename... Spans>
    class ZipIterator {
        std::tuple<Spans...> spans;
        std::size_t index;
    public:
        using value_type = std::tuple<typename Spans::value_type&...>;

        ZipIterator(std::tuple<Spans...> spans, std::size_t index)
            : spans(spans), index(index) {}

        auto operator*() const {
            return std::apply([this](auto&... s) {
                return std::tie(s[index]...);
            }, spans);
        }

        ZipIterator& operator++() {
            index++;
            return *this;
        }

        bool operator==(const ZipIterator& other) const {
            return index == other.index;
        }
    }; */

    export template <QueriedComponentType... QueriedComponents>
    requires (sizeof...(QueriedComponents) > 0)
    class Query {
    public:
        struct Chunk {
            std::tuple<std::span<std::remove_reference_t<QueriedComponents>>...> data;
        };
    private:
        struct SortedQuery {
            std::vector<std::string_view> immutable_components;
            std::vector<std::string_view> mutable_components;
            std::vector<std::string_view> immutable_resources;
            std::vector<std::string_view> mutable_resources;
        };

        std::vector<Chunk> chunks;
    public:
        explicit Query() {}

        explicit Query(std::span<std::span<uint8_t>> data) {
            chunks.resize(data.size());
            for (int i = 0; i < data.size(); i++) {
                [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                    ([&]<std::size_t I>() {
                        using Component = std::tuple_element_t<I, std::tuple<QueriedComponents...>>;

                        std::get<I>(chunks[i].data) = std::span{reinterpret_cast<std::remove_reference_t<Component>*>(data[i + I].data()), data[i + I].size() / sizeof(std::remove_reference_t<Component>)};
                    }.template operator()<Is>(), ...);
                }(std::index_sequence_for<QueriedComponents...>{});
            }
        }

        auto single() {
            return std::apply([](auto&... spans) {
                return std::forward_as_tuple(spans[0]...);
            }, chunks[0].data);
        }

        /* auto single() {
            return std::apply([](auto&... spans) {
                return std::forward_as_tuple(spans[0]...);
            }, data);
        }

        auto single() const {
            return std::apply([](auto&... spans) {
                return std::forward_as_tuple(spans[0]...);
            }, data);
        }

        auto begin() { return ZipIterator(data, 0); }
        auto end() { return ZipIterator(data, std::get<0>(data).size()); }

        auto begin() const { return ZipIterator(data, 0); }
        auto end() const { return ZipIterator(data, std::get<0>(data).size()); } */
    };
}
