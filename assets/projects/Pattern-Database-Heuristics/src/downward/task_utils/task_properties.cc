#include "downward/task_utils/task_properties.h"

#include "downward/utils/logging.h"
#include "downward/utils/memory.h"
#include "downward/utils/system.h"

#include "downward/state.h"

#include <algorithm>
#include <iostream>
#include <limits>

using namespace std;
using utils::ExitCode;

namespace task_properties {

bool is_applicable(const AbstractOperatorProxy& op, const State& state)
{
    for (FactProxy precondition : op.get_preconditions()) {
        if (state[precondition.get_variable()] != precondition.get_value())
            return false;
    }
    return true;
}

bool is_goal_state(const AbstractPlanningTask& task, const State& state)
{
    for (FactProxy goal : task.get_goal()) {
        if (state[goal.get_variable()] != goal.get_value()) return false;
    }
    return true;
}

bool is_unit_cost(const ClassicalPlanningTask& task)
{
    for (OperatorProxy op : task.get_operators()) {
        if (op.get_cost() != 1) return false;
    }
    return true;
}

vector<int> get_operator_costs(const ClassicalPlanningTask& task_proxy)
{
    vector<int> costs;
    OperatorsProxy operators = task_proxy.get_operators();
    costs.reserve(operators.size());
    for (OperatorProxy op : operators)
        costs.push_back(op.get_cost());
    return costs;
}

double get_average_operator_cost(const ClassicalPlanningTask& task_proxy)
{
    double average_operator_cost = 0;
    for (OperatorProxy op : task_proxy.get_operators()) {
        average_operator_cost += op.get_cost();
    }
    average_operator_cost /= task_proxy.get_operators().size();
    return average_operator_cost;
}

int get_min_operator_cost(const ClassicalPlanningTask& task_proxy)
{
    int min_cost = numeric_limits<int>::max();
    for (OperatorProxy op : task_proxy.get_operators()) {
        min_cost = min(min_cost, op.get_cost());
    }
    return min_cost;
}

int get_num_facts(const AbstractPlanningTask& task_proxy)
{
    int num_facts = 0;
    for (VariableProxy var : task_proxy.get_variables())
        num_facts += var.get_domain_size();
    return num_facts;
}

int get_num_total_effects(const ClassicalPlanningTask& task_proxy)
{
    int num_effects = 0;
    for (OperatorProxy op : task_proxy.get_operators())
        num_effects += op.get_effects().size();
    return num_effects;
}

void print_variable_statistics(
    const AbstractPlanningTask& task_proxy,
    utils::LogProxy& log)
{
    const int_packer::IntPacker& state_packer = g_state_packers[task_proxy];

    int num_facts = 0;
    VariablesProxy variables = task_proxy.get_variables();
    for (VariableProxy var : variables)
        num_facts += var.get_domain_size();

    log << "Variables: " << variables.size() << endl;
    log << "FactPairs: " << num_facts << endl;
    log << "Bytes per state: "
        << state_packer.get_num_bins() * sizeof(PackedStateBin) << endl;
}

void dump_pddl(const AbstractPlanningTask& task_proxy, const State& state)
{
    for (FactProxy fact : task_proxy.get_state_facts(state)) {
        string fact_name = fact.get_name();
        if (fact_name != "<none of those>") utils::g_log << fact_name << endl;
    }
}

void dump_fdr(const AbstractPlanningTask& task_proxy, const State& state)
{
    for (FactProxy fact : task_proxy.get_state_facts(state)) {
        VariableProxy var = fact.get_variable();
        utils::g_log << "  #" << var.get_id() << " [" << var.get_name()
                     << "] -> " << fact.get_value() << endl;
    }
}

void dump_goals(const GoalProxy& goals)
{
    utils::g_log << "Goal conditions:" << endl;
    for (FactProxy goal : goals) {
        utils::g_log << "  " << goal.get_variable().get_name() << ": "
                     << goal.get_value() << endl;
    }
}

void dump_task(const ClassicalPlanningTask& task_proxy)
{
    OperatorsProxy operators = task_proxy.get_operators();
    int min_action_cost = numeric_limits<int>::max();
    int max_action_cost = 0;
    for (OperatorProxy op : operators) {
        min_action_cost = min(min_action_cost, op.get_cost());
        max_action_cost = max(max_action_cost, op.get_cost());
    }
    utils::g_log << "Min action cost: " << min_action_cost << endl;
    utils::g_log << "Max action cost: " << max_action_cost << endl;

    VariablesProxy variables = task_proxy.get_variables();
    utils::g_log << "Variables (" << variables.size() << "):" << endl;
    for (VariableProxy var : variables) {
        utils::g_log << "  " << var.get_name() << " (range "
                     << var.get_domain_size() << ")" << endl;
        for (int val = 0; val < var.get_domain_size(); ++val) {
            utils::g_log << "    " << val << ": "
                         << var.get_fact(val).get_name() << endl;
        }
    }
    State initial_state = task_proxy.get_initial_state();
    utils::g_log << "Initial state (PDDL):" << endl;
    dump_pddl(task_proxy, initial_state);
    utils::g_log << "Initial state (FDR):" << endl;
    dump_fdr(task_proxy, initial_state);
    dump_goals(task_proxy.get_goal());
}

PerTaskInformation<int_packer::IntPacker>
    g_state_packers([](const AbstractPlanningTask& task_proxy) {
        VariablesProxy variables = task_proxy.get_variables();
        vector<int> variable_ranges;
        variable_ranges.reserve(variables.size());
        for (VariableProxy var : variables) {
            variable_ranges.push_back(var.get_domain_size());
        }
        return std::make_unique<int_packer::IntPacker>(variable_ranges);
    });
} // namespace task_properties
