#ifndef PROBFD_VALUE_TYPE_H
#define PROBFD_VALUE_TYPE_H

#include <limits>
#include <string>

namespace probfd {

/// The type used for probabilities.
using value_t = double;

/// A constant representing number \f$+\infty\f$.
static constexpr value_t INFINITE_VALUE =
    std::numeric_limits<double>::infinity();

/// The default tolerance for approximate comparisons.
extern value_t g_epsilon;

value_t abs(value_t val);

constexpr value_t double_to_value(double d)
{
    return d;
}

/// User-defined literals for value_t.
constexpr value_t operator""_vt(long double value)
{
    return double_to_value(static_cast<double>(value));
}

/// User-defined literals for value_t.
constexpr value_t operator""_vt(unsigned long long value)
{
    return double_to_value(static_cast<double>(value));
}

/// Equivalent to \f$|v_1 - v_2| \leq \epsilon\f$
bool is_approx_equal(value_t v1, value_t v2, value_t epsilon = g_epsilon);

/// Equivalent to \f$v_2 - v_1 > \epsilon\f$
bool is_approx_less(value_t v1, value_t v2, value_t epsilon = g_epsilon);

/// Equivalent to \f$v_1 - v_2 > \epsilon\f$
bool is_approx_greater(value_t v1, value_t v2, value_t epsilon = g_epsilon);

/// Returns unary function for approximate inequality comparison.
constexpr auto approx_eq_fun(value_t epsilon = g_epsilon)
{
    return [=](value_t left, value_t right) {
        return is_approx_equal(left, right, epsilon);
    };
}

/// Returns unary function for approximate inequality comparison.
constexpr auto approx_neq_fun(value_t epsilon = g_epsilon)
{
    return [=](value_t left, value_t right) {
        return !is_approx_equal(left, right, epsilon);
    };
}

/// Returns unary function for approximate inequality comparison.
constexpr auto approx_less_fun(value_t epsilon = g_epsilon)
{
    return [=](value_t left, value_t right) {
        return is_approx_less(left, right, epsilon);
    };
}

} // namespace probfd

#endif // PROBFD_VALUE_TYPE_H
