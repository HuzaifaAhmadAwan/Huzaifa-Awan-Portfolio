#include "downward/state.h"

#include "downward/state_registry.h"

#include "downward/task_utils/task_properties.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>

using namespace std;

State::State(
    const StateRegistry& registry,
    StateID id,
    const PackedStateBin* buffer)
    : registry(&registry)
    , id(id)
    , buffer(buffer)
    , values(nullptr)
    , state_packer(&registry.get_state_packer())
    , num_variables(registry.get_num_variables())
{
    assert(id != StateID::no_state);
    assert(buffer);
}

State::State(vector<int> values)
    : registry(nullptr)
    , id(StateID::no_state)
    , buffer(nullptr)
    , values(make_shared<vector<int>>(std::move(values)))
    , state_packer(nullptr)
    , num_variables(this->values->size())
{
}

bool operator==(const State& left, const State& right)
{
    if (left.registry && right.registry) {
        // Both states are registered and from the same registry. Compare IDs.
        return left.id == right.id;
    }

    // Compare values directly.
    if (!left.values) {
        auto left_range = std::views::iota(0, left.num_variables) |
                          std::views::transform([&](int var) {
                              return left.state_packer->get(left.buffer, var);
                          });
        if (!right.values) {
            auto right_range =
                std::views::iota(0, left.num_variables) |
                std::views::transform([&](int var) {
                    return right.state_packer->get(right.buffer, var);
                });
            return std::ranges::equal(left_range, right_range);
        } else {
            return std::ranges::equal(left_range, *right.values);
        }
    } else if (!right.values) {
        auto right_range =
            std::views::iota(0, left.num_variables) |
            std::views::transform([&](int var) {
                return right.state_packer->get(right.buffer, var);
            });
        return std::ranges::equal(*left.values, right_range);
    }

    return *left.values == *right.values;
}

std::strong_ordering operator<=>(const State& left, const State& right)
{
    if (left.registry && right.registry) {
        // Both states are registered and from the same registry. Compare IDs.
        return left.id <=> right.id;
    }

    // Compare values directly.
    if (!left.values) {
        auto left_range = std::views::iota(0, left.num_variables) |
                          std::views::transform([&](int var) {
                              return left.state_packer->get(left.buffer, var);
                          });
        if (!right.values) {
            auto right_range =
                std::views::iota(0, left.num_variables) |
                std::views::transform([&](int var) {
                    return right.state_packer->get(right.buffer, var);
                });
            return std::lexicographical_compare_three_way(
                left_range.begin(),
                left_range.end(),
                right_range.begin(),
                right_range.end());
        } else {
            return std::lexicographical_compare_three_way(
                left_range.begin(),
                left_range.end(),
                right.values->begin(),
                right.values->end());
        }
    } else if (!right.values) {
        auto right_range =
            std::views::iota(0, left.num_variables) |
            std::views::transform([&](int var) {
                return right.state_packer->get(right.buffer, var);
            });
        return std::lexicographical_compare_three_way(
            left.values->begin(),
            left.values->end(),
            right_range.begin(),
            right_range.end());
    }

    return *left.values <=> *right.values;
}

void State::unpack() const
{
    if (!values) {
        int num_variables = size();
        /*
          A micro-benchmark in issue348 showed that constructing the vector
          in the required size and then assigning values was faster than the
          more obvious reserve/push_back. Although, the benchmark did not
          profile this specific code.

          We might consider a bulk-unpack method in state_packer that could be
          more efficient. (One can imagine state packer to have extra data
          structures that exploit sequentially unpacking each entry, by doing
          things bin by bin.)
        */
        values = std::make_shared<std::vector<int>>(num_variables);
        for (int var = 0; var < num_variables; ++var) {
            (*values)[var] = state_packer->get(buffer, var);
        }
    }
}

std::size_t State::size() const
{
    return num_variables;
}

int State::operator[](std::size_t var_id) const
{
    assert(var_id < size());
    if (values) {
        return (*values)[var_id];
    } else {
        assert(buffer);
        assert(state_packer);
        return state_packer->get(buffer, var_id);
    }
}

int State::operator[](VariableProxy var) const
{
    return (*this)[var.get_id()];
}

const StateRegistry* State::get_registry() const
{
    return registry;
}

StateID State::get_id() const
{
    return id;
}

const PackedStateBin* State::get_buffer() const
{
    /*
      TODO: we should profile what happens if we #ifndef NDEBUG this test here
      and in other places (e.g. the next method). The 'if' itself is probably
      not costly, but the 'cerr <<' stuff might prevent inlining.
    */
    if (!buffer) {
        std::cerr << "Accessing the packed values of an unregistered state is "
                  << "treated as an error." << std::endl;
        utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
    }
    return buffer;
}

const std::vector<int>& State::get_unpacked_values() const
{
    if (!values) {
        std::cerr << "Accessing the unpacked values of a state without "
                  << "unpacking them first is treated as an error. Please "
                  << "use State::unpack first." << std::endl;
        utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
    }
    return *values;
}

State get_unregistered_successor(const State& state, const OperatorProxy& op)
{
    assert(task_properties::is_applicable(op, state));
    return get_unregistered_successor(state, op.get_effects());
}

namespace utils {
void feed(HashState& hash_state, const State& state)
{
    /*
      Hashing a state without unpacked data will result in an error.
      We don't want to unpack states implicitly, so this rules out the option
      of unpacking the states here on demand. Mixing hashes from packed and
      unpacked states would lead to logically equal states with different
      hashes. Hashing packed (and therefore registered) states also seems like
      a performance error because it's much cheaper to hash the state IDs
      instead.
    */
    feed(hash_state, state.get_unpacked_values());
}

} // namespace utils