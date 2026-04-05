#ifndef DOWNWARD_TASKS_SYNTACTIC_PROJECTION_H
#define DOWNWARD_TASKS_SYNTACTIC_PROJECTION_H

#include "downward/abstract_task.h"
#include "downward/task_proxy.h"

#include "downward/pdbs/types.h"

#include <string>
#include <vector>

namespace tasks {

/**
 * @brief Represents the syntactic projection of a planning task.
 *
 * The syntactic projection of a planning task \f$\Pi = (V, A, I, G)\f$ with
 * respect to a subset of variables \f$P \subseteq V\f$ is the planning task
 * \f$\Pi|_P = (P, A|_P, I|_P, G|_P)\f$ where
 * \f$A|_P = \{ a|_P \mid a \in A\}\f$
 * with \f$\mathit{cost}(a|_P) = \mathit{cost}(a)\f$,
 * \f$\mathit{pre}(a|_P) = \mathit{pre}(a)|_P\f$ and
 * \f$\mathit{eff}(a|_P) = \mathit{eff}(a)|_P\f$.
 *
 * The syntactic projection only considers the variable in \f$P\f$.
 * The operators \f$A|_P\f$ are copies of the operators of the original planning
 * task, in which the variables \f$v \notin P\f$ are removed from the
 * preconditions and effects, effectively ignoring them.
 *
 * @ingroup planning_task_impls
 */
class SyntacticProjection : public ClassicalPlanningTask {
    struct ExplicitVariable {
        std::string name;
        int domain_size;
        int original_var;
    };

    struct ExplicitOperator {
        std::string name;
        int cost;
        std::vector<FactPair> precondition;
        std::vector<FactPair> effect;
    };

    std::vector<std::vector<std::string>> fact_names;

    std::vector<ExplicitVariable> variables;
    std::vector<ExplicitOperator> operators;
    std::vector<int> initial_state_values;
    std::vector<FactPair> goal_facts;

public:
    /**
     * @name Constructors
     */
    ///@{
    /// @brief Constructs the syntactic projection \f$\Pi|_P\f$ of an input
    /// planning task \f$ \Pi \f$ with respect to the input pattern \f$ P \f$.
    ///
    /// The variables of the syntactic projection are ordered in the same way
    /// as specified by the input sorted pattern, i.e., if \p pattern = [2, 3]
    /// after sorting variables indices by increasing order, then variable 0
    /// corresponds to variable 2 and variable 1 corresponds to variable 3 of
    /// \p parent_task .
    explicit SyntacticProjection(
        const ClassicalPlanningTask& parent_task,
        const pdbs::Pattern& pattern);
    ///@}

    /**
     * @name Abstract State Computation
     */
    ///@{
    /// Converts a concrete state of the ancestor task to its corresponding
    /// abstract state of the syntactic projection.
    ///
    /// Time complexity: \f$ \mathcal{O}(|P|) \f$
    State compute_abstract_state(const State& concrete_state);
    ///@}

    int get_num_variables() const override;

    std::string get_variable_name(int var) const override;

    int get_variable_domain_size(int var) const override;

    std::string get_fact_name(const FactPair& fact) const override;

    bool are_facts_mutex(const FactPair& fact1, const FactPair& fact2)
        const override;

    int get_num_operators() const override;

    std::string get_operator_name(int op_index) const override;

    int get_operator_cost(int op_index) const override;

    int get_num_operator_preconditions(int op_index) const override;

    FactPair
    get_operator_precondition(int op_index, int fact_index) const override;

    int get_num_operator_effects(int op_index) const override;

    FactPair get_operator_effect(int op_index, int eff_index) const override;

    int get_num_goals() const override;

    FactPair get_goal_fact(int index) const override;

    std::vector<int> get_initial_state_values() const override;
};

} // namespace tasks

#endif
