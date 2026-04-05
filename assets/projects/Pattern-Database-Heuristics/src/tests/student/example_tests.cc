#include <gtest/gtest.h>

#include "downward/utils/extended_int.h"

TEST(StudentTests, trivial_test)
{
    // Succeeds unconditionally.
    SUCCEED();
}

TEST(StudentTests, undefined_operation_test)
{
    try {
        // inf - inf is an undefined operation that is supposed to throw a
        // std::domain_error. Let's check if this is the case...
        ExtendedInt l = ExtendedInt::infinity() - ExtendedInt::infinity();
        (void) l;
    } catch (const std::domain_error&) {
        return; // ok, pass test by returning.
    }

    FAIL() << "Expected std::domain_error to be thrown!";
}

TEST(StudentTests, overflow_test)
{
    try {
        // Provoke an overflow. This is supposed to throw a std::overflow_error.
        // Let's check if this is the case...
        ExtendedInt l = ExtendedInt(std::numeric_limits<int>::max()) + 1;
        (void) l;
    } catch (const std::overflow_error&) {
        return; // ok, pass test by returning.
    }

    FAIL() << "Expected std::overflow_error to be thrown!";
}