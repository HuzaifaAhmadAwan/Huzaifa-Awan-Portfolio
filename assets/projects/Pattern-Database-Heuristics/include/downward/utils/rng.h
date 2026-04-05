#ifndef UTILS_RNG_H
#define UTILS_RNG_H

#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <vector>

namespace utils {
class RandomNumberGenerator {
    // Mersenne Twister random number generator.
    std::mt19937 rng;

public:
    RandomNumberGenerator(); // Seed with a value depending on time and process
                             // ID.
    explicit RandomNumberGenerator(int seed);
    RandomNumberGenerator(const RandomNumberGenerator&) = delete;
    RandomNumberGenerator& operator=(const RandomNumberGenerator&) = delete;
    ~RandomNumberGenerator();

    void seed(int seed);

    // Return random double in [0..1).
    double random();

    // Return random integer in [0..bound).
    int random(int bound);

    double get_uniform(std::uniform_int_distribution<int>& dist);

    double gaussian(double mean = 0.0, double stddev = 1.0)
    {
        std::normal_distribution<double> distribution(mean, stddev);
        return distribution(rng);
    }

    template <typename T>
    typename T::const_iterator choose(const T& vec)
    {
        return vec.begin() + random(vec.size());
    }

    template <typename T>
    typename T::iterator choose(T& vec)
    {
        return vec.begin() + random(vec.size());
    }

    template <typename T>
    void shuffle(std::vector<T>& vec)
    {
        std::shuffle(vec.begin(), vec.end(), rng);
    }
};

template <typename W>
size_t
weighted_choose_index(RandomNumberGenerator& rng, const std::vector<W>& weights)
{
    assert(
        all_of(weights.begin(), weights.end(), [](W i) { return i >= 0.0; }));
    double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
    assert(sum > 0.0);
    double choice = rng.random() * sum;
    for (size_t i = 0; i < weights.size(); ++i) {
        choice -= weights[i];
        if (choice < 0) {
            return i;
        }
    }
    assert(false);
    return 0;
}

template <typename T, typename W>
typename std::vector<T>::const_iterator weighted_choose(
    RandomNumberGenerator& rng,
    const std::vector<T>& vec,
    const std::vector<W>& weights)
{
    assert(vec.size() == weights.size());
    return vec.begin() + weighted_choose_index(rng, weights);
}

std::vector<int> choose_n_of_N(RandomNumberGenerator& rng, int n, int N);

} // namespace utils

#endif
