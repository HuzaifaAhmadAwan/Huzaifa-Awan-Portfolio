#ifndef DOWNWARD_STATE_H
#define DOWNWARD_STATE_H

#include "downward/concepts.h"
#include "downward/state_id.h"

#include "downward/algorithms/int_packer.h"

#include "downward/utils/hash.h"

#include <compare>
#include <cstddef>
#include <iterator>
#include <memory>
#include <ranges>
#include <vector>

class FactProxy;
class VariableProxy;
class OperatorProxy;

class StateRegistry;

/**
 * @brief Represents a state of a planning task \f$\task\f$, i.e. a complete
 * variable assignment.
 *
 * The State class contains the individual variable values
 * \f$s(v), v \in \varspaceof{\task}\f$ of a state \f$s\f$ of \f$\task\f$.
 * Variable values of a state can be accessed individually using operator[] with
 * a variable index.
 * Alternatively, all variable values can be returned as a list.
 * You can iterate over all facts of a state by obtaining a range of facts via
 * the method const AbstractPlanningTask&::get_state_facts:
 * ```cpp
 * for (FactProxy fact : task.get_state_facts(state)) {
 *     // Do something with the fact...
 * }
 * ```
 *
 * States can be created directly by specifying the complete variable assignment
 * as a list of values.
 * Alternatively, they can be created as successor states of other states as
 * the result of applying an operator's effect.
 *
 * @see get_unregistered_successor
 *
 * @ingroup states
 */
class State {
    const StateRegistry* registry;
    StateID id;
    const PackedStateBin* buffer;

    mutable std::shared_ptr<std::vector<int>> values;
    const int_packer::IntPacker* state_packer;
    int num_variables;

    /* Access the packed values. Accessing packed values on states that do
   not have them (unregistered states) is an error. */
    const PackedStateBin* get_buffer() const;

    /* Return a pointer to the registry in which this state is registered.
   If the state is not registered, return nullptr. */
    const StateRegistry* get_registry() const;

    // Construct a registered state.
    State(
        const StateRegistry& registry,
        StateID id,
        const PackedStateBin* buffer);

    // Friend Declarations
    friend class StateRegistry;
    friend class AbstractPlanningTask;
    template <typename>
    friend class PerStateArray;
    template <typename>
    friend class PerStateInformation;

    friend std::strong_ordering
    operator<=>(const State& left, const State& right);

    friend bool operator==(const State& left, const State& right);

    friend State
    get_unregistered_successor(const State& state, const OperatorProxy& op);

    friend State get_unregistered_successor(
        const State& state,
        const InputRange<FactProxy> auto& effects);

public:
    /**
     * @name Constructors & Destructors
     * @{
     */

    /**
     * @brief Construct a state of a planning task that represents the given
     * complete variable assignment.
     *
     * The entry `values[i]` of the input vector corresponds to the value
     * assigned to the variable with index `i`.
     *
     * @warning The size of the vector should match the number of variables
     * of the planning task this object is tied to.
     */
    explicit State(std::vector<int> values);

    /**
     * @}
     */

    /**
     * @name Accessing Variable Values
     * These member functions can be used to access the values \f$s(v)\f$ for
     * the variables \f$v \in \vars\f$.
     * @{
     */

    /// Get the number of variables \f$|\varspaceof{\task}|\f$ of the task
    /// \f$\task\f$ this state belongs to.
    std::size_t size() const;

    /// Get the domain value \f$s(v)\f$ assigned to variable
    /// \f$v \in \varspaceof{\task}\f$ by this state, given by its unique index
    /// in range \f$\{ 0, \dots, |\varspaceof{\task}| - 1\}\f$.
    int operator[](std::size_t var_id) const;

    /// Get the domain value \f$s(v)\f$ assigned to variable
    /// \f$v \in \varspaceof{\task}\f$ by this state.
    int operator[](VariableProxy var) const;

    /**
     * @brief Access the state's variable assignment as a vector.
     *
     * The entry `values[i]` of the vector corresponds to the value assigned
     * to the variable with index `i`.
     */
    const std::vector<int>& get_unpacked_values() const;

    /**
     * @}
     */

    // Internal method used by the search algorithms.
    StateID get_id() const;

    // Internal method used by the search algorithms.
    void unpack() const;
};

/// Compares two states lexicographically.
std::strong_ordering operator<=>(const State& left, const State& right);

/// Checks if two states are equal.
bool operator==(const State& left, const State& right);

/**
 * @brief Create a successor state by applying the effect of a given
 * operator to a given state.
 *
 * Equivalent to `get_unregistered_successor(state, op.get_effects());`.
 */
State get_unregistered_successor(const State& state, const OperatorProxy& op);

/**
 * @brief Create a successor state by applying the given effect to a
 * given state.
 *
 * The effect parameter must model a range of FactProxy values, e.g.,
 * OperatorEffectProxy, OperatorPreconditionProxy or GoalProxy.
 * The parameter is interpreted as a partial variable assignment \f$e\f$
 * and the function returns the successor state
 * \f[
 *     s'(v) :=
 *     \begin{cases}
 *         s(v) & v \notin \vars[e], \\
 *         e(v) & v \in \vars[e].
 *     \end{cases}
 * \f]
 *
 * @see StateRegistry::get_successor
 *
 * @note If the parameter contains multiple facts \f$(v, d_1), (v, d_2)\f$
 * for a variable \f$v\f$, then only the last fact is considered and
 * all other facts do not influence the result of the computation.
 */
State get_unregistered_successor(
    const State& state,
    const InputRange<FactProxy> auto& effects)
{
    std::vector<int> new_values = state.get_unpacked_values();

    for (const auto effect : effects) {
        new_values[effect.get_variable().get_id()] = effect.get_value();
    }

    return State(std::move(new_values));
}

namespace utils {
void feed(HashState& hash_state, const State& state);
} // namespace utils

#endif
