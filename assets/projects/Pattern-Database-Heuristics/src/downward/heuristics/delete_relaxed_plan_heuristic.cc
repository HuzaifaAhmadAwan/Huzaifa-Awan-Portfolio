#include "downward/heuristics/delete_relaxed_plan_heuristic.h"

int DeleteRelaxedPlanHeuristic::compute_heuristic(const State& state)
{
    auto plan = this->compute_relaxed_plan(state);
    return plan.has_value() ? plan->cost : Heuristic::DEAD_END;
}
