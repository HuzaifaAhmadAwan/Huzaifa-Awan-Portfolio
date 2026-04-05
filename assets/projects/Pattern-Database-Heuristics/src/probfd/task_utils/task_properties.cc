#include "probfd/task_utils/task_properties.h"

#include "probfd/probabilistic_task.h"

#include "downward/task_utils/task_properties.h"

#include "downward/utils/logging.h"
#include "downward/utils/system.h"

#include "downward/state.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <ranges>
#include <unordered_map>
#include <utility>

using namespace std;
namespace vws = std::views;
using utils::ExitCode;

namespace probfd::task_properties {

static bool is_one(value_t value)
{
    return value == 1_vt;
}

bool is_applicable(const ProbabilisticOperatorProxy& op, const State& state)
{
    for (FactProxy precondition : op.get_preconditions()) {
        if (state[precondition.get_variable()] != precondition.get_value())
            return false;
    }
    return true;
}

bool is_unit_cost(const ProbabilisticPlanningTask& task)
{
    return std::ranges::all_of(
        task.get_operators() |
            vws::transform(&ProbabilisticOperatorProxy::get_cost),
        is_one);
}

value_t get_adjusted_action_cost(
    const ProbabilisticOperatorProxy& op,
    OperatorCost cost_type,
    bool is_unit_cost)
{
    switch (cost_type) {
    case NORMAL: return op.get_cost();
    case ONE: return 1_vt;
    case PLUSONE:
        if (is_unit_cost)
            return 1_vt;
        else
            return op.get_cost() + 1_vt;
    default: ABORT("Unknown cost type");
    }
}

vector<value_t> get_operator_costs(const ProbabilisticPlanningTask& task)
{
    vector<value_t> costs;
    ProbabilisticOperatorsProxy operators = task.get_operators();
    costs.reserve(operators.size());
    for (ProbabilisticOperatorProxy op : operators)
        costs.push_back(op.get_cost());
    return costs;
}

value_t get_average_operator_cost(const ProbabilisticPlanningTask& task)
{
    value_t average_operator_cost = 0;
    for (ProbabilisticOperatorProxy op : task.get_operators()) {
        average_operator_cost += op.get_cost();
    }
    return average_operator_cost /
           static_cast<value_t>(task.get_operators().size());
}

value_t get_min_operator_cost(const ProbabilisticPlanningTask& task)
{
    value_t min_cost = INFINITE_VALUE;
    for (ProbabilisticOperatorProxy op : task.get_operators()) {
        min_cost = min(min_cost, op.get_cost());
    }
    return min_cost;
}

int get_num_total_effects(const ProbabilisticPlanningTask& task)
{
    int num_effects = 0;
    for (ProbabilisticOperatorProxy op : task.get_operators())
        for (ProbabilisticOutcomeProxy outcome : op.get_outcomes())
            num_effects += outcome.get_effects().size();
    return num_effects;
}

void dump_probabilistic_task(const ProbabilisticPlanningTask& task)
{
    ProbabilisticOperatorsProxy operators = task.get_operators();
    value_t min_action_cost = numeric_limits<int>::max();
    value_t max_action_cost = 0;
    for (ProbabilisticOperatorProxy op : operators) {
        min_action_cost = min(min_action_cost, op.get_cost());
        max_action_cost = max(max_action_cost, op.get_cost());
    }
    utils::g_log << "Min action cost: " << min_action_cost << endl;
    utils::g_log << "Max action cost: " << max_action_cost << endl;

    VariablesProxy variables = task.get_variables();
    utils::g_log << "Variables (" << variables.size() << "):" << endl;
    for (VariableProxy var : variables) {
        utils::g_log << "  " << var.get_name() << " (range "
                     << var.get_domain_size() << ")" << endl;
        for (int val = 0; val < var.get_domain_size(); ++val) {
            utils::g_log << "    " << val << ": "
                         << var.get_fact(val).get_name() << endl;
        }
    }
    State initial_state = task.get_initial_state();
    utils::g_log << "Initial state (PDDL):" << endl;
    ::task_properties::dump_pddl(task, initial_state);
    utils::g_log << "Initial state (FDR):" << endl;
    ::task_properties::dump_fdr(task, initial_state);
    ::task_properties::dump_goals(task.get_goal());
}

} // namespace probfd::task_properties
