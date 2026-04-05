#ifndef PROBFD_PROBABILISTIC_TASK_H
#define PROBFD_PROBABILISTIC_TASK_H

#include "downward/abstract_task.h" // IWYU pragma: export

#include "probfd/value_type.h"

namespace probfd {

class ProbabilisticOperatorsProxy;

/**
 * @brief Represents a probabilistic planning task.
 *
 * @attention The virtual methods of this class are intentionally left
 * undocumented as they should not be used directly!
 * *
 */
class ProbabilisticPlanningTask : public AbstractPlanningTask {
public:
    /// Get the cost of the probabilistic operator with index \p op_index.
    virtual value_t get_operator_cost(int op_index) const = 0;

    /// Get the number of probabilistic outcomes of the probabilistic operator
    /// with index \p op_index.
    virtual int get_num_operator_outcomes(int op_index) const = 0;

    /// Get the probabilistic outcome probability of the outcome with index
    /// \p outcome_index of the probabilistic operator with index \p op_index.
    virtual value_t
    get_operator_outcome_probability(int op_index, int outcome_index) const = 0;

    /// Get the global outcome index for the outcome with index \p outcome_index
    /// of the probabilistic operator with index \p op_index.
    virtual int
    get_operator_outcome_id(int op_index, int outcome_index) const = 0;

    /// Get the number of effects of the outcome with index \p outcome_index of
    /// the probabilistic operator with index \p op_index.
    virtual int
    get_num_operator_outcome_effects(int op_index, int outcome_index) const = 0;

    /// Get the effect with index \p eff_index of the outcome with index
    /// \p outcome_index of the probabilistic operator with index \p op_index.
    virtual FactPair
    get_operator_outcome_effect(int op_index, int outcome_index, int eff_index)
        const = 0;

    /// Returns the set of probabilistic operators.
    ProbabilisticOperatorsProxy get_operators() const;
};

} // namespace probfd

#endif
