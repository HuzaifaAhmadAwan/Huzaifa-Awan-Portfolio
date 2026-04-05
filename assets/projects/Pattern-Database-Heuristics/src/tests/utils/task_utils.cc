#include "tests/utils/task_utils.h"

#include "tests/domains/classical_planning_domain.h"

#include "probfd/probabilistic_task.h"

#include "downward/abstract_task.h"

#include "downward/utils/system.h"

#include <algorithm>

using namespace probfd;

namespace {

class ProbabilisticWrapperTask : public ProbabilisticPlanningTask {
    std::shared_ptr<ClassicalPlanningTask> deterministic_task;

public:
    explicit ProbabilisticWrapperTask(
        std::shared_ptr<ClassicalPlanningTask> deterministic_task)
        : deterministic_task(std::move(deterministic_task))
    {
    }

    int get_num_variables() const override
    {
        return deterministic_task->get_num_variables();
    }

    std::string get_variable_name(int var) const override
    {
        return deterministic_task->get_variable_name(var);
    }

    int get_variable_domain_size(int var) const override
    {
        return deterministic_task->get_variable_domain_size(var);
    }

    std::string get_fact_name(const FactPair& fact) const override
    {
        return deterministic_task->get_fact_name(fact);
    }

    bool
    are_facts_mutex(const FactPair& fact1, const FactPair& fact2) const override
    {
        return deterministic_task->are_facts_mutex(fact1, fact2);
    }

    std::string get_operator_name(int index) const override
    {
        return deterministic_task->get_operator_name(index);
    }

    int get_num_operators() const override
    {
        return deterministic_task->get_num_operators();
    }

    int get_num_operator_preconditions(int index) const override
    {
        return deterministic_task->get_num_operator_preconditions(index);
    }

    FactPair
    get_operator_precondition(int op_index, int fact_index) const override
    {
        return deterministic_task->get_operator_precondition(
            op_index,
            fact_index);
    }

    int get_num_goals() const override
    {
        return deterministic_task->get_num_goals();
    }

    FactPair get_goal_fact(int index) const override
    {
        return deterministic_task->get_goal_fact(index);
    }

    std::vector<int> get_initial_state_values() const override
    {
        return deterministic_task->get_initial_state_values();
    }

    value_t get_operator_cost(int op_index) const override
    {
        return static_cast<value_t>(
            deterministic_task->get_operator_cost(op_index));
    }

    int get_num_operator_outcomes(int) const override { return 1; }

    value_t get_operator_outcome_probability(int, int) const override
    {
        return 1.0_vt;
    }

    int get_operator_outcome_id(int op_index, int) const override
    {
        return op_index;
    }

    int get_num_operator_outcome_effects(int op_index, int) const override
    {
        return deterministic_task->get_num_operator_effects(op_index);
    }

    FactPair
    get_operator_outcome_effect(int op_index, int, int eff_index) const override
    {
        return deterministic_task->get_operator_effect(op_index, eff_index);
    }
};

class DomainBasedTask : public ClassicalPlanningTask {
    const tests::ClassicalPlanningDomain& domain;
    std::vector<int> initial_state;
    std::vector<FactPair> goal;

public:
    DomainBasedTask(
        const tests::ClassicalPlanningDomain& domain,
        std::vector<int> initial,
        std::vector<FactPair> goal)
        : domain(domain)
        , initial_state(std::move(initial))
        , goal(std::move(goal))
    {
    }

    int get_num_variables() const override
    {
        return domain.get_num_variables();
    }

    std::string get_variable_name(int var) const override
    {
        return domain.get_variable_name(var);
    }

    int get_variable_domain_size(int var) const override
    {
        return domain.get_variable_domain_size(var);
    }

    std::string get_fact_name(const FactPair& fact) const override
    {
        return domain.get_fact_name(fact);
    }

    int get_num_operators() const override
    {
        return domain.get_num_operators();
    }

    int get_operator_cost(int index) const override
    {
        return domain.get_operator_cost(index);
    }

    std::string get_operator_name(int index) const override
    {
        return domain.get_operator_name(index);
    }

    int get_num_operator_preconditions(int index) const override
    {
        return domain.get_num_operator_preconditions(index);
    }

    FactPair get_operator_precondition(int index, int fact_index) const override
    {
        return domain.get_operator_precondition(index, fact_index);
    }

    int get_num_operator_effects(int index) const override
    {
        return domain.get_num_operator_effects(index);
    }

    FactPair get_operator_effect(int index, int fact_index) const override
    {
        return domain.get_operator_effect(index, fact_index);
    }

    int get_num_goals() const override { return goal.size(); }

    FactPair get_goal_fact(int index) const override { return goal[index]; }

    std::vector<int> get_initial_state_values() const override
    {
        return initial_state;
    }

    // Default implementations for inapplicable parts of the interface

    bool are_facts_mutex(const FactPair&, const FactPair&) const override
    {
        return false;
    }
};

} // namespace

namespace tests {

std::shared_ptr<ClassicalPlanningTask> create_task_from_domain(
    const ClassicalPlanningDomain& domain,
    const std::vector<FactPair>& initial,
    std::vector<FactPair> goal)
{
    if (!domain.verify_complete_assignment(initial)) {
        ABORT("Initial state is not a complete variable assignment!");
    }

    if (!domain.verify_partial_assignment(initial)) {
        ABORT("Initial state is not a partial variable assignment!");
    }

    std::vector<int> state(initial.size());
    for (const auto& [var, val] : initial) {
        state[var] = val;
    }

    std::ranges::sort(goal);

    return std::make_shared<DomainBasedTask>(
        domain,
        std::move(state),
        std::move(goal));
}

std::unique_ptr<ProbabilisticPlanningTask>
to_probabilistic_task(std::shared_ptr<ClassicalPlanningTask> deterministic_task)
{
    return std::make_unique<ProbabilisticWrapperTask>(
        std::move(deterministic_task));
}

}