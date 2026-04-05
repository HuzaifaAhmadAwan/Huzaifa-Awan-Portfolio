#include "tests/utils/pdb_utils.h"

#include "downward/heuristic.h"
#include "downward/task_proxy.h"

#include "downward/task_utils/task_properties.h"

#include <algorithm>

namespace tests {

pdbs::Pattern get_greedy_pattern(
    const ClassicalPlanningTask& task_proxy, int abstract_state_limit)
{
    VariablesProxy variables = task_proxy.get_variables();

    // Collect goal and non-goal variables in (stable) order of domain size.
    std::vector<int> all_vars;
    std::vector<int> goal_vars;
    std::vector<int> non_goal_vars;

    for (VariableProxy var : task_proxy.get_variables()) {
        all_vars.push_back(var.get_id());
    }

    for (FactProxy goal_fact : task_proxy.get_goal()) {
        goal_vars.push_back(goal_fact.get_variable().get_id());
    }

    std::ranges::set_difference(
        all_vars,
        goal_vars,
        std::back_inserter(non_goal_vars));

    auto cmp_domain_size = [&](int var, int var2) {
        return variables[var].get_domain_size() <
               variables[var2].get_domain_size();
    };

    std::ranges::stable_sort(goal_vars, cmp_domain_size);
    std::ranges::stable_sort(non_goal_vars, cmp_domain_size);

    pdbs::Pattern pattern;
    int num_abstract_states = 1;

    // Add as many goal variables in increasing order of domain size as possible
    for (int var : goal_vars) {
        int new_num_abstract_states =
            num_abstract_states * variables[var].get_domain_size();
        if (new_num_abstract_states > abstract_state_limit) return pattern;
        num_abstract_states = new_num_abstract_states;
        pattern.push_back(var);
    }

    // If the limit has not been exceeded to the same for non-goal variables
    for (int var : non_goal_vars) {
        int new_num_abstract_states =
            num_abstract_states * variables[var].get_domain_size();
        if (new_num_abstract_states > abstract_state_limit) return pattern;
        num_abstract_states = new_num_abstract_states;
        pattern.push_back(var);
    }

    return pattern;
}

pdbs::PatternCollection get_atomic_patterns(const ClassicalPlanningTask& task_proxy)
{
    pdbs::PatternCollection patterns;
    for (auto fact : task_proxy.get_goal()) {
        patterns.push_back({fact.get_variable().get_id()});
    }
    return patterns;
}

}