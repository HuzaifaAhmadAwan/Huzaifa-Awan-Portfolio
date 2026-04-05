#include "downward/tasks/syntactic_projection.h"

#include "downward/task_utils/task_properties.h"

#include "downward/utils/collections.h"

#include "downward/state.h"
#include "downward/task_proxy.h"

#include <unordered_set>
#include <unordered_map>

namespace tasks {

SyntacticProjection::SyntacticProjection(
    const ClassicalPlanningTask& parent_task,
    const pdbs::Pattern& p)
{
    std::unordered_set<int> pattern(p.begin(), p.end());

    const ClassicalPlanningTask& proxy(parent_task);

    VariablesProxy vars = proxy.get_variables();

    std::unordered_map<int, int> global2local;

    auto parent_init_values = parent_task.get_initial_state_values();

    int idx = 0;
    for (int var : pattern) {
        global2local[var] = idx++;
        auto& var_fact_names = fact_names.emplace_back();

        int domain_size = vars[var].get_domain_size();

        for (int i = 0; i != domain_size; ++i) {
            var_fact_names.push_back(
                parent_task.get_fact_name(FactPair(var, i)));
        }

        ExplicitVariable& new_var = variables.emplace_back();
        new_var.name = vars[var].get_name();
        new_var.domain_size = domain_size;
        new_var.original_var = var;

        initial_state_values.push_back(parent_init_values[var]);
    }

    for (OperatorProxy op : proxy.get_operators()) {
        ExplicitOperator& new_op = operators.emplace_back();
        new_op.name = op.get_name();
        new_op.cost = op.get_cost();

        for (FactProxy fact : op.get_preconditions()) {
            if (auto it = global2local.find(fact.get_variable().get_id());
                it != global2local.end()) {
                new_op.precondition.emplace_back(it->second, fact.get_value());
            }
        }

        for (const auto& fact : op.get_effects()) {
            if (auto it = global2local.find(fact.get_variable().get_id());
                it != global2local.end()) {
                new_op.effect.emplace_back(it->second, fact.get_value());
            }
        }
    }

    for (FactProxy fact : proxy.get_goal()) {
        if (auto it = global2local.find(fact.get_variable().get_id());
            it != global2local.end()) {
            goal_facts.emplace_back(it->second, fact.get_value());
        }
    }
}

State SyntacticProjection::compute_abstract_state(const State& concrete_state)
{
    std::vector<int> abstract_values(variables.size());

    for (std::size_t i = 0; i != variables.size(); ++i) {
        const int orig_var = variables[i].original_var;
        abstract_values[i] = concrete_state[orig_var];
    }

    return State(std::move(abstract_values));
}

int SyntacticProjection::get_num_variables() const
{
    return variables.size();
}

std::string SyntacticProjection::get_variable_name(int var) const
{
    return variables[var].name;
}

int SyntacticProjection::get_variable_domain_size(int var) const
{
    return variables[var].domain_size;
}

std::string SyntacticProjection::get_fact_name(const FactPair& fact) const
{
    return fact_names[fact.var][fact.value];
}

bool SyntacticProjection::are_facts_mutex(const FactPair&, const FactPair&)
    const
{
    abort();
}

int SyntacticProjection::get_num_operators() const
{
    return operators.size();
}

std::string SyntacticProjection::get_operator_name(int op_index) const
{
    return operators[op_index].name;
}

int SyntacticProjection::get_operator_cost(int op_index) const
{
    return operators[op_index].cost;
}

int SyntacticProjection::get_num_operator_preconditions(int op_index) const
{
    return operators[op_index].precondition.size();
}

FactPair
SyntacticProjection::get_operator_precondition(int op_index, int fact_index)
    const
{
    return operators[op_index].precondition[fact_index];
}

int SyntacticProjection::get_num_operator_effects(int op_index) const
{
    return operators[op_index].effect.size();
}

FactPair
SyntacticProjection::get_operator_effect(int op_index, int eff_index) const
{
    return operators[op_index].effect[eff_index];
}

int SyntacticProjection::get_num_goals() const
{
    return goal_facts.size();
}

FactPair SyntacticProjection::get_goal_fact(int index) const
{
    return goal_facts[index];
}

std::vector<int> SyntacticProjection::get_initial_state_values() const
{
    return initial_state_values;
}

} // namespace tasks