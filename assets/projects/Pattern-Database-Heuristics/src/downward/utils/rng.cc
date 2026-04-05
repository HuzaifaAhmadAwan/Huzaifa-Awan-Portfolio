#include "downward/utils/rng.h"

#include "downward/utils/system.h"

#include <chrono>

#define PORTABLE_RNG

using namespace std;

namespace utils {
/*
  Ideally, one would use true randomness here from std::random_device. However,
  there exist platforms where this returns non-random data, which is condoned by
  the standard. On these platforms one would need to additionally seed with time
  and process ID (PID), and therefore generally seeding with time and PID only
  is probably good enough.
*/
RandomNumberGenerator::RandomNumberGenerator()
{
    unsigned int secs = static_cast<unsigned int>(
        chrono::system_clock::now().time_since_epoch().count());
    seed(secs + get_process_id());
}

RandomNumberGenerator::RandomNumberGenerator(int seed_)
{
    seed(seed_);
}

RandomNumberGenerator::~RandomNumberGenerator()
{
}

double RandomNumberGenerator::random()
{
#ifndef PORTABLE_RNG
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(rng);
#else
    /*
     * Microsoft's uniform_real_distribution implementation.
     */

    auto iterations = [](const uint64_t Gmin, const uint64_t Gmax) {
        // For a URBG type with range == `(Gmax - Gmin) +
        // 1`, returns the number of calls to generate at least Bits
        // bits of entropy. Specifically, max(1, ceil(Bits /
        // log2(range))).
        constexpr int Bits = std::numeric_limits<double>::digits;

        if (Gmax == std::numeric_limits<uint64_t>::max() && Gmin == 0) {
            return 1;
        }

        const auto Range = (Gmax - Gmin) + 1;
        const auto Target = ~uint64_t{0} >> (64 - Bits);
        uint64_t Prod = 1;
        int Ceil = 0;
        while (Prod <= Target) {
            ++Ceil;
            if (Prod > UINT64_MAX / Range) {
                break;
            }

            Prod *= Range;
        }

        return Ceil;
    };

    constexpr auto Gxmin = static_cast<double>(std::mt19937::min());
    constexpr auto Gxmax = static_cast<double>(std::mt19937::max());
    constexpr auto Rx = (Gxmax - Gxmin) + 1.0;

    constexpr int Kx = iterations(std::mt19937::min(), std::mt19937::max());

    double Ans = 0.0;
    double Factor = 1.0;

    for (int Idx = 0; Idx < Kx; ++Idx) { // add in another set of bits
        Ans += (static_cast<double>(rng()) - Gxmin) * Factor;
        Factor *= Rx;
    }

    return Ans / Factor;
#endif
}

// Return random integer in [0..bound).
int RandomNumberGenerator::random(int bound)
{
    assert(bound > 0);

#ifndef PORTABLE_RNG
    std::uniform_int_distribution<int> distribution(0, bound - 1);
    return distribution(rng);
#else
    /*
     * Microsoft's uniform_int_distribution implementation.
     */

    constexpr unsigned int uint_bits = sizeof(unsigned int) * 8;
    static_assert(uint_bits <= 32);

    using Uprod = std::conditional_t<uint_bits <= 16, uint32_t, uint64_t>;

    constexpr auto Calc_bits = [] {
        auto Bits_local = uint_bits;
        auto Bmask_local = static_cast<unsigned int>(-1);
        for (; std::mt19937::max() - std::mt19937::min() < Bmask_local;
             Bmask_local >>= 1) {
            --Bits_local;
        }

        return Bits_local;
    };

    constexpr size_t Bits = Calc_bits();
    constexpr unsigned int Bmask =
        static_cast<unsigned int>(-1) >> (uint_bits - Bits); // 2^Bits - 1

    auto Adjust = [](unsigned int Uval) noexcept {
        // convert signed ranges to unsigned ranges and vice versa
        constexpr unsigned int Adjuster =
            (static_cast<unsigned int>(-1) >> 1) + 1; // 2^(N-1)

        if (Uval < Adjuster) {
            return static_cast<unsigned int>(Uval + Adjuster);
        } else {
            return static_cast<unsigned int>(Uval - Adjuster);
        }
    };

    auto Get_bits = [&]() {
        // return a random value within [0, Bmask]
        for (;;) { // repeat until random value is in range
            const unsigned int Val = rng() - std::mt19937::min();

            if (Val <= Bmask) {
                return Val;
            }
        }
    };

    auto Get_all_bits = [&]() {
        unsigned int Ret = Get_bits();

        if constexpr (Bits < uint_bits) {
            for (unsigned int Num = Bits; Num < uint_bits;
                 Num += Bits) { // don't mask away any bits
                Ret <<= Bits;
                Ret |= Bits;
            }
        }

        return Ret;
    };

    auto do_it = [&](unsigned int Index) {
        auto Get_random_product = [&](unsigned int Index) {
            unsigned int Ret = Get_bits();
            static_assert(Bits >= uint_bits);
            /*{
                while (--Niter > 0) {
                    Ret <<= Bits;
                    Ret |= Get_bits();
                }
            }*/
            return Uprod{Ret} * Uprod{Index};
        };

        unsigned int Mask = Bmask;
        // unsigned int Niter = 1;

        if constexpr (Bits < uint_bits) {
            while (Mask < static_cast<unsigned int>(Index - 1)) {
                Mask <<= Bits;
                Mask |= Bmask;
                //        ++Niter;
            }
        }

        // x <- random integer in [0, 2^L)
        // m <- x * s
        auto Product = Get_random_product(Index /*, Niter*/);
        // l <- m mod 2^L
        auto Rem = static_cast<unsigned int>(Product) & Mask;

        if (Rem < Index) {
            // t <- (2^L - s) mod s
            const auto Threshold = (Mask - Index + 1) % Index;
            while (Rem < Threshold) {
                Product = Get_random_product(Index /*, Niter*/);
                Rem = static_cast<unsigned int>(Product) & Mask;
            }
        }

        unsigned int Generated_bits;
        if constexpr (Bits < uint_bits) {
            Generated_bits = static_cast<unsigned int>(std::popcount(Mask));
        } else {
            Generated_bits = uint_bits;
        }

        // m / 2^L
        return static_cast<unsigned int>(Product >> Generated_bits);
    };

    const unsigned int Umin = Adjust(0U);
    const unsigned int Umax = Adjust(static_cast<unsigned int>(bound - 1));

    unsigned int Uret;

    if (Umax - Umin == static_cast<unsigned int>(-1)) {
        Uret = static_cast<unsigned int>(Get_all_bits());
    } else {
        Uret = do_it(Umax - Umin + 1U);
    }

    return static_cast<int>(Adjust(static_cast<unsigned int>(Uret + Umin)));
#endif
}

double
RandomNumberGenerator::get_uniform(std::uniform_int_distribution<int>& dist)
{
#ifndef PORTABLE_RNG
    return dist(rng);
#else
    return random() * (dist.max() - dist.min()) + dist.min();
#endif
}

void RandomNumberGenerator::seed(int seed)
{
    rng.seed(seed);
}

vector<int> choose_n_of_N(RandomNumberGenerator& rng, int n, int N)
{
    vector<int> result;
    result.reserve(n);
    for (int i = 0; i < n; ++i) {
        int r;
        do {
            r = rng.random(N);
        } while (find(std::begin(result), std::end(result), r) !=
                 std::end(result));
        result.push_back(r);
    }
    return result;
}

} // namespace utils
