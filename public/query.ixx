module;

#include <tuple>
#include <span>

export module specs.query;

import specs.component;

namespace specs {
    template <typename... Spans>
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
    };

    export template <ComponentType... QueriedComponents>
    class Query {
    private:
        std::tuple<std::span<QueriedComponents>...> data;
    public:
        auto single() {
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
        auto end() const { return ZipIterator(data, std::get<0>(data).size()); }
    };
}