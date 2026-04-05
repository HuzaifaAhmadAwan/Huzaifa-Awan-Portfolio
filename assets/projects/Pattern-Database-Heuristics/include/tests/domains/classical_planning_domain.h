#ifndef TESTS_DOMAINS_CLASSICAL_PLANNING_DOMAIN_H
#define TESTS_DOMAINS_CLASSICAL_PLANNING_DOMAIN_H

#include "downward/abstract_task.h"

#include <algorithm>
#include <string>
#include <vector>

namespace tests {

/**
 * @brief Models a variable space and a set of operators which together
 * model a probabilistic planning domain.
 *
 * @ingroup test_domains
 */
class ClassicalPlanningDomain {
protected:
    struct VariableInfo {
        std::string name;
        int domain_size;
        std::vector<std::string> fact_names;
    };

    struct OperatorInfo {
        std::string name;
        int cost;
        std::vector<FactPair> precondition;
        std::vector<FactPair> effect;

        OperatorInfo() = default;

        OperatorInfo(
            std::string name,
            int cost,
            std::vector<FactPair> precondition,
            std::vector<FactPair> effect)
            : name(std::move(name))
            , cost(cost)
            , precondition(std::move(precondition))
            , effect(std::move(effect))
        {
            std::ranges::sort(precondition);
            std::ranges::sort(effect);
        }
    };

    std::vector<VariableInfo> variable_infos;
    std::vector<OperatorInfo> operators;

public:
    ClassicalPlanningDomain() = default;
    ClassicalPlanningDomain(int num_variables, int num_operators);

    int get_num_variables() const;
    std::string get_variable_name(int var) const;
    int get_variable_domain_size(int var) const;
    std::string get_fact_name(const FactPair& fact) const;

    int get_num_operators() const;
    std::string get_operator_name(int index) const;
    int get_operator_cost(int index) const;

    int get_num_operator_preconditions(int op_index) const;
    FactPair get_operator_precondition(int op_index, int fact_index) const;

    int get_num_operator_effects(int op_index) const;
    FactPair get_operator_effect(int op_index, int eff_index) const;

    bool verify_partial_assignment(const std::vector<FactPair>& facts) const;
    bool verify_complete_assignment(const std::vector<FactPair>& facts) const;
};

} // namespace tests

#endif