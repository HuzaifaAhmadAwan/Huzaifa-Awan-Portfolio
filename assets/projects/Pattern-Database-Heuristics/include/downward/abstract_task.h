#ifndef ABSTRACT_TASK_H
#define ABSTRACT_TASK_H

#include "downward/fact_pair.h"
#include "downward/operator_id.h"

#include "downward/algorithms/int_packer_types.h"
#include "downward/algorithms/subscriber.h"
#include "downward/utils/hash.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

// Forward Declarations
class OperatorsProxy;
class VariablesProxy;
class FactsProxy;
class AbstractOperatorsProxy;
class State;
class GoalProxy;
class StateFactsProxy;
class ClassicalPlanningTask;

class TaskID;
class StateID;
class StateRegistry;

namespace causal_graph {
class CausalGraph;
}

/*
 * @brief Represents a general planning task that is either classical or
 * probabilistic.
 *
 * A general planning task is a tuple
 * \f$\task = (\varspace, \operators, \initstate, \taskgoal)\f$, where
 * \f$\varspace\f$ is a variable space, \f$\operators\f$ is a finite set of
 * partial operators, each associated with a precondition, but not with a cost
 * or effect(s), \f$\initstate\f$ is an initial state, i.e., a complete variable
 * assignment with respect to \f$\varspace\f$ and \f$\taskgoal\f$ is a partial
 * variable assignment.
 *
 * This class serves as a base class for the sub-classes ClassicalPlanningTask
 * and ProbabilisticPlanningTask which model classical and probabilistic
 * planning tasks respectively.
 * These classes also associate integer or floating-point costs with each
 * operator, as well as deterministic or probabilistic effects.
 *
 * @attention The virtual methods of this class are intentionally left
 * undocumented as they should not be used directly!
 *
 * @see ClassicalPlanningTask
 * @see ProbabilisticPlanningTask
 *
 * @ingroup planning_tasks
 */
class AbstractPlanningTask
    : public subscriber::SubscriberService<AbstractPlanningTask> {
public:
    virtual int get_num_variables() const = 0;
    virtual std::string get_variable_name(int var) const = 0;
    virtual int get_variable_domain_size(int var) const = 0;
    virtual std::string get_fact_name(const FactPair& fact) const = 0;
    virtual bool
    are_facts_mutex(const FactPair& fact1, const FactPair& fact2) const = 0;

    virtual std::string get_operator_name(int index) const = 0;
    virtual int get_num_operators() const = 0;
    virtual int get_num_operator_preconditions(int index) const = 0;
    virtual FactPair
    get_operator_precondition(int op_index, int fact_index) const = 0;

    virtual int get_num_goals() const = 0;
    virtual FactPair get_goal_fact(int index) const = 0;

    virtual std::vector<int> get_initial_state_values() const = 0;

    /// Get the set of variables \f$\varspaceof{\task}\f$ of the planning task.
    VariablesProxy get_variables() const;

    /// Get the set of all facts \f$\factsof{\task}\f$ of the planning task.
    FactsProxy get_facts() const;

    // Get the set of abstract operators \f$\operatorsof{\task}\f$ of the
    // planning task.
    AbstractOperatorsProxy get_abstract_operators() const;

    /// Creates the initial state \f$\initstateof{\task}\f$ of the planning
    /// task.
    State get_initial_state() const;

    /// Get the goal \f$\goalof{\task}\f$ of the planning task.
    GoalProxy get_goal() const;

    /**
     * @brief Get the set of facts induced by a state of the planning task.
     *
     * @warning The returned StateFactsProxy takes a reference to the passed
     * state. Do not pass a temporary to this function.
     * ```
     * // Incorrect:
     * StateFactsProxy p = task.get_state_facts(task.get_initial_state());
     * // The temporary task.get_initial_state() is destroyed at this
     * // point, so p will refer to a dead object. Accessing p will cause
     * // undefined behaviour.
     *
     * // Correct:
     * const State state = task.get_initial_state();
     * StateFactsProxy p = task.get_state_facts(state);
     * ```
     */
    StateFactsProxy get_state_facts(const State& state) const;

    /// Creates a state of this planning task with the specified values.
    State create_state(std::vector<int>&& state_values) const;

    // Internal method.
    void subscribe_to_task_destruction(
        subscriber::Subscriber<AbstractPlanningTask>* subscriber) const;

    // Internal method.
    TaskID get_id() const;

private:
    friend class StateRegistry;

    // This method is meant to be called only by the state registry.
    State create_state(
        const StateRegistry& registry,
        StateID id,
        const PackedStateBin* buffer) const;
};

/**
 * @brief Represents a classical planning task.
 *
 * A classical planning task is a tuple
 * \f$\task = (\varspace, \operators, \initstate, \taskgoal)\f$, where
 * \f$\varspace\f$ is a variable space, \f$\operators\f$ is a finite set of
 * deterministic operators, where each \f$a \in \operators\f$ is associated with
 * a precondition \f$\pre_a\f$, an effect \f$\eff_a\f$ and
 * a cost \f$\cost(a)\f$ (assumed to be an integer by this implementation),
 * \f$\initstate\f$ is an initial state, i.e., a complete variable
 * assignment with respect to \f$\varspace\f$ and \f$\taskgoal\f$ is a partial
 * variable assignment.
 *
 * Usage example:
 * ```
 * bool has_zero_cost_operator(const ClassicalPlanningTask& task) {
 *     // Iterate over all operators
 *     for (const OperatorProxy& op : task.get_operators()) {
 *         // Access the cost of the operator
 *         int cost = op.get_cost();
 *         if (cost == 0) return true;
 *     }
 *
 *     return false;
 * }
 * ```
 *
 * @attention The virtual methods of this class are intentionally left
 * undocumented as they should not be used directly!
 *
 * @see State
 *
 * @ingroup planning_tasks
 */
class ClassicalPlanningTask : public AbstractPlanningTask {
public:
    virtual int get_operator_cost(int index) const = 0;

    virtual int get_num_operator_effects(int op_index) const = 0;
    virtual FactPair get_operator_effect(int op_index, int eff_index) const = 0;

    /// Get the set of operators \f$\operatorsof{\task}\f$ of the classical
    /// planning task.
    OperatorsProxy get_operators() const;

    // Internal method.
    const causal_graph::CausalGraph& get_causal_graph() const;
};

#endif
