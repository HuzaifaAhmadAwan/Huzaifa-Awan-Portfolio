
#ifndef HEURISTICS_DELETE_RELAXED_PLAN_HEURISTIC_H
#define HEURISTICS_DELETE_RELAXED_PLAN_HEURISTIC_H

#include "downward/heuristic.h"
#include "downward/operator_id.h"

#include <optional>
#include <vector>

class State;

/**
 * @brief Represents a delete-relaxed plan, paired with its cost.
 *
 * @ingroup heuristics
 */
struct DeleteRelaxedPlan {
    /// The operator sequence forming the delete-relaxed plan.
    std::vector<OperatorID> operators;

    /// The total cost of the operator sequence.
    int cost;
};

/**
 * @brief Represents a heuristic derived from a delete-relaxed plan.
 *
 * @ingroup heuristics
 */
class DeleteRelaxedPlanHeuristic : public Heuristic {
public:
    using Heuristic::Heuristic; // inherit constructors

    virtual ~DeleteRelaxedPlanHeuristic() = default;

    int compute_heuristic(const State& state) override;

    /**
     * @brief Returns a delete-relaxed plan for the state paired with its cost,
     * if existent, and std::nullopt otherwise.
     *
     * @note Implementations are allowed to be non-deterministic, i.e., to
     * return different delete-relaxed plans when called multiple times on the
     * same state.
     *
     * @see DeleteRelaxedPlan
     */
    virtual std::optional<DeleteRelaxedPlan>
    compute_relaxed_plan(const State& state) = 0;
};

#endif
