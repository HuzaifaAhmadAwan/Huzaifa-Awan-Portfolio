#include "probfd/value_type.h"

#include <cassert>
#include <cmath>

namespace probfd {

value_t g_epsilon = 5e-5;

value_t abs(value_t val)
{
    return std::abs(val);
}

bool is_approx_equal(value_t v1, value_t v2, value_t tolerance)
{
    assert(tolerance >= 0.0_vt);
    return v1 == v2 || std::abs(v1 - v2) <= tolerance;
}

bool is_approx_less(value_t v1, value_t v2, value_t tolerance)
{
    assert(tolerance >= 0.0_vt);
    return v1 + tolerance < v2;
}

bool is_approx_greater(value_t v1, value_t v2, value_t tolerance)
{
    assert(tolerance >= 0.0_vt);
    return v1 - tolerance > v2;
}

} // namespace probfd
