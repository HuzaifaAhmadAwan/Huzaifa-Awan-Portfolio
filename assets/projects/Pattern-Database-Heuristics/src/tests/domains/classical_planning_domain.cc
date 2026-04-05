#include "tests/domains/classical_planning_domain.h"

#include "downward/utils/system.h"

#include <set>

namespace tests {

namespace {

bool in_domain(int val, int high)
{
    return val >= 0 && val < high;
}

} // namespace

ClassicalPlanningDomain::ClassicalPlanningDomain(
    int num_variables,
    int num_operators)
    : variable_infos(num_variables)
    , operators(num_operators)
{
}

int ClassicalPlanningDomain::get_num_variables() const
{
    return variable_infos.size();
}

std::string ClassicalPlanningDomain::get_variable_name(int var) const
{
    return variable_infos[var].name;
}

int ClassicalPlanningDomain::get_variable_domain_size(int var) const
{
    return variable_infos[var].domain_size;
}

std::string ClassicalPlanningDomain::get_fact_name(const FactPair& fact) const
{
    return variable_infos[fact.var].fact_names[fact.value];
}

int ClassicalPlanningDomain::get_num_operators() const
{
    return operators.size();
}

std::string ClassicalPlanningDomain::get_operator_name(int index) const
{
    return operators[index].name;
}

int ClassicalPlanningDomain::get_operator_cost(int index) const
{
    return operators[index].cost;
}

int ClassicalPlanningDomain::get_num_operator_preconditions(int op_index) const
{
    return operators[op_index].precondition.size();
}

FactPair
ClassicalPlanningDomain::get_operator_precondition(int op_index, int fact_index)
    const
{
    return operators[op_index].precondition[fact_index];
}

int ClassicalPlanningDomain::get_num_operator_effects(int op_index) const
{
    return operators[op_index].effect.size();
}

FactPair
ClassicalPlanningDomain::get_operator_effect(int op_index, int eff_index) const
{
    return operators[op_index].effect[eff_index];
}

bool ClassicalPlanningDomain::verify_partial_assignment(
    const std::vector<FactPair>& facts) const
{
    std::set<int> vars;

    for (const auto& [var, val] : facts) {
        if (!vars.insert(var).second ||
            !in_domain(val, get_variable_domain_size(var)))
            return false;
    }

    return true;
}

bool ClassicalPlanningDomain::verify_complete_assignment(
    const std::vector<FactPair>& facts) const
{
    return facts.size() == static_cast<size_t>(get_num_variables()) &&
           verify_partial_assignment(facts);
}

} // namespace tests
