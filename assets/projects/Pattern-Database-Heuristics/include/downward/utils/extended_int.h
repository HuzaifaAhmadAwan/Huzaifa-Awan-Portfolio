#ifndef DOWNWARD_UTILS_INFINITY_EXTENDED_H
#define DOWNWARD_UTILS_INFINITY_EXTENDED_H

#include <concepts>
#include <iostream>
#include <stdexcept>

#if defined(__clang__) || defined(__GNUC__)
template <std::integral I>
struct arith_helper {
    static bool add(I a, I b, I* c) { return __builtin_add_overflow(a, b, c); }

    static bool sub(I a, I b, I* c) { return __builtin_sub_overflow(a, b, c); }

    static bool mult(I a, I b, I* c) { return __builtin_mul_overflow(a, b, c); }
};
#elif defined(_MSC_VER)

#define ENABLE_INTSAFE_SIGNED_FUNCTIONS
#include <intsafe.h>

template <std::integral I>
struct selector;

template <>
struct selector<unsigned char> {
    static constexpr auto add = UInt8Add;
    static constexpr auto sub = UInt8Sub;
    static constexpr auto mult = UInt8Mult;
};

template <>
struct selector<unsigned short int> {
    static constexpr auto add = UShortAdd;
    static constexpr auto sub = UShortSub;
    static constexpr auto mult = UShortMult;
};

template <>
struct selector<unsigned int> {
    static constexpr auto add = UIntAdd;
    static constexpr auto sub = UIntSub;
    static constexpr auto mult = UIntMult;
};

template <>
struct selector<unsigned long int> {
    static constexpr auto add = ULongAdd;
    static constexpr auto sub = ULongSub;
    static constexpr auto mult = ULongMult;
};

template <>
struct selector<unsigned long long int> {
    static constexpr auto add = ULongLongAdd;
    static constexpr auto sub = ULongLongSub;
    static constexpr auto mult = ULongLongMult;
};

template <>
struct selector<signed char> {
    static constexpr auto add = Int8Add;
    static constexpr auto sub = Int8Sub;
    static constexpr auto mult = Int8Mult;
};

template <>
struct selector<short int> {
    static constexpr auto add = ShortAdd;
    static constexpr auto sub = ShortSub;
    static constexpr auto mult = ShortMult;
};

template <>
struct selector<int> {
    static constexpr auto add = IntAdd;
    static constexpr auto sub = IntSub;
    static constexpr auto mult = IntMult;
};

template <>
struct selector<long int> {
    static constexpr auto add = LongAdd;
    static constexpr auto sub = LongSub;
    static constexpr auto mult = LongMult;
};

template <>
struct selector<long long int> {
    static constexpr auto add = LongLongAdd;
    static constexpr auto sub = LongLongSub;
    static constexpr auto mult = LongLongMult;
};

template <std::integral I>
struct arith_helper {
    static bool add(I a, I b, I* c)
    {
        return selector<I>::add(a, b, c) != S_OK;
    }

    static bool sub(I a, I b, I* c)
    {
        return selector<I>::sub(a, b, c) != S_OK;
    }

    static bool mult(I a, I b, I* c)
    {
        return selector<I>::mult(a, b, c) != S_OK;
    }
};

#else
#error "Unsupported compiler!"
#endif

template <std::integral I>
class ExtendedInteger;

template <std::integral I>
bool operator<(ExtendedInteger<I> left, ExtendedInteger<I> right);

template <std::integral I>
bool operator==(ExtendedInteger<I> left, ExtendedInteger<I> right);

template <std::integral I>
class ExtendedInteger {
    I value_;
    bool is_infinite_;

    ExtendedInteger(std::convertible_to<I> auto value, bool is_infinite)
        : value_(value)
        , is_infinite_(is_infinite)
    {
    }

public:
    ExtendedInteger(std::convertible_to<I> auto integer)
        : value_(integer)
        , is_infinite_(false)
    {
    }

    explicit operator I() const
    {
        if (is_infinite())
            throw std::domain_error(
                "Cannot cast infinite value to integer type!");
        return value_;
    }

    static ExtendedInteger<I> infinity() { return ExtendedInteger<I>{1, true}; }

    static ExtendedInteger<I> negative_infinity() { return {-1, true}; }

    bool is_finite() const { return !is_infinite(); }
    bool is_infinite() const { return is_infinite_; }

    bool is_positive_infinite() const { return is_infinite() && value_ > 0; }
    bool is_negative_infinite() const { return is_infinite() && value_ < 0; }

    friend ExtendedInteger<I>
    operator+(ExtendedInteger<I> left, ExtendedInteger<I> right)
    {
        if (left.is_negative_infinite()) {
            if (right.is_positive_infinite())
                throw std::domain_error("Undefined operation: -inf + inf!");

            return negative_infinity();
        }

        if (left.is_positive_infinite()) {
            if (right.is_negative_infinite())
                throw std::domain_error("Undefined operation: inf + -inf!");

            return infinity();
        }

        if (right.is_negative_infinite()) return negative_infinity();
        if (right.is_positive_infinite()) return infinity();

        I result;
        if (arith_helper<I>::add(left.value_, right.value_, &result)) {
            throw std::overflow_error("Addition overflow occured!");
        }

        return ExtendedInteger<I>(result);
    }

    friend ExtendedInteger<I>
    operator-(ExtendedInteger<I> left, ExtendedInteger<I> right)
    {
        if (left.is_negative_infinite()) {
            if (right.is_negative_infinite())
                throw std::domain_error("Undefined operation: -inf - (-inf)!");

            return negative_infinity();
        }

        if (left.is_positive_infinite()) {
            if (right.is_positive_infinite())
                throw std::domain_error("Undefined operation: inf - inf!");

            return infinity();
        }

        if (right.is_negative_infinite()) return infinity();
        if (right.is_positive_infinite()) return negative_infinity();

        I result;
        if (arith_helper<I>::sub(left.value_, right.value_, &result)) {
            throw std::overflow_error("Substraction overflow occured!");
        }

        return ExtendedInteger<I>(left.value_ - right.value_);
    }

    friend ExtendedInteger<I>
    operator*(ExtendedInteger<I> left, ExtendedInteger<I> right)
    {
        if (left.value_ == 0 || right.value_ == 0) return ExtendedInteger<I>(0);

        if (left.is_negative_infinite()) {
            if (right.value_ > 0) return negative_infinity();
            return infinity();
        }

        if (left.is_positive_infinite()) {
            if (right.value_ > 0) return infinity();
            return negative_infinity();
        }

        if (right.is_negative_infinite()) return negative_infinity();
        if (right.is_positive_infinite()) return infinity();

        I result;
        if (arith_helper<I>::mult(left.value_, right.value_, &result)) {
            throw std::overflow_error("Multiplication overflow occured!");
        }

        return ExtendedInteger<I>(left.value_ * right.value_);
    }

    friend bool operator<(ExtendedInteger<I> left, ExtendedInteger<I> right)
    {
        if (left.is_negative_infinite()) return !right.is_negative_infinite();
        return !left.is_infinite() &&
               (right.is_infinite() ||
                (!right.is_negative_infinite() && left.value_ < right.value_));
    }

    friend bool
    operator==(ExtendedInteger<I> left, ExtendedInteger<I> right) = default;

    friend std::ostream& operator<<(std::ostream& os, ExtendedInteger<I> right)
    {
        if (right.is_infinite()) {
            if (right.value_ < 0) os << "-";
            return os << "inf";
        }

        return os << right.value_;
    }
};

using ExtendedInt = ExtendedInteger<int>;
using ExtendedUInt = ExtendedInteger<unsigned int>;

#endif // DOWNWARD_UTILS_INFINITY_EXTENDED_H
