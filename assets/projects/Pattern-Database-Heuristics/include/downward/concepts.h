
#ifndef DOWNWARD_CONCEPTS_H
#define DOWNWARD_CONCEPTS_H

#include <concepts>
#include <cstddef>
#include <ranges>

template <typename T>
concept SizedSubscriptable = requires(const T c, std::size_t s) {
    c.operator[](s);
    { c.size() } -> std::convertible_to<std::size_t>;
};

/// Models an input range R that yields values of type V.
template <typename R, typename V>
concept InputRange = std::ranges::input_range<R> &&
                     std::convertible_to<std::ranges::range_value_t<R>, V>;

#endif
