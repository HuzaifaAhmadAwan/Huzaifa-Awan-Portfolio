
#include "downward/pdbs/state_enumerator.h"

#include "downward/utils/math.h"

#include "downward/state.h"
#include "downward/task_proxy.h"

#include <cstdlib>
#include <limits>
#include <utility>

namespace pdbs {

StateEnumerator::StateEnumerator(
    const std::shared_ptr<ClassicalPlanningTask>& task)
    : task_(task)
{
    std::size_t multiplier = 1;
    for (const auto& var : task->get_variables()) {
        multipliers_.emplace_back(multiplier);
        if (!utils::is_product_within_limit(
                multiplier,
                var.get_domain_size(),
                std::numeric_limits<int>::max())) {
            throw std::overflow_error("Total number of states exceeds "
                                      "std::numeric_limits<int>::max()!");
        }
        multiplier *= var.get_domain_size();
    }

    num_states_ = multiplier;
}

std::size_t StateEnumerator::get_index(const State& state) const
{
    std::size_t index = 0;

    for (std::size_t i = 0; i != state.size(); ++i) {
        index += multipliers_[i] * state[i];
    }

    return index;
}

State StateEnumerator::get_state(std::size_t index) const
{
    std::vector<int> values(multipliers_.size(), -1);

    for (std::size_t i = values.size(); i-- > 0;) {
        const auto& [quot, rem] = std::div(
            static_cast<int>(index),
            static_cast<int>(multipliers_[i]));
        values[i] = quot;
        index = rem;
    }

    return State(std::move(values));
}

std::size_t StateEnumerator::num_states() const
{
    return num_states_;
}

} // namespace pdbs
