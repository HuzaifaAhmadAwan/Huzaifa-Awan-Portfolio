#ifndef DOWNWARD_TASK_PROXY_H
#define DOWNWARD_TASK_PROXY_H

#include "downward/concepts.h"

#include "downward/abstract_task.h"
#include "downward/operator_id.h"
#include "downward/state_id.h"
#include "downward/task_id.h"

#include "downward/algorithms/int_packer.h"

#include <compare>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

class FactProxy;
class FactsProxy;
class GoalProxy;
class OperatorEffectProxy;
class OperatorProxy;
class OperatorsProxy;
class AbstractOperatorsProxy;
class OperatorPreconditionProxy;
class State;
class StateRegistry;
class VariableProxy;
class VariablesProxy;

namespace probfd {
class ProbabilisticEffectProxy;
}

namespace causal_graph {
class CausalGraph;
}

template <SizedSubscriptable T>
class ProxyIterator {
    /* We store a pointer to collection instead of a reference
       because iterators have to be copy assignable. */
    const T* collection = nullptr;
    std::size_t pos = -1;

public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type =
        typename std::remove_cvref_t<decltype(std::declval<T>().operator[](
            0U))>;
    using difference_type = int;
    using pointer = const value_type*;
    using reference = value_type;

    ProxyIterator() = default;

    ProxyIterator(const T& collection, std::size_t pos)
        : collection(&collection)
        , pos(pos)
    {
    }

    reference operator*() const { return (*collection)[pos]; }

    ProxyIterator<T> operator++(int)
    {
        auto r = *this;
        ++(*this);
        return r;
    }

    ProxyIterator<T> operator--(int)
    {
        auto r = *this;
        --(*this);
        return r;
    }

    ProxyIterator<T>& operator++()
    {
        ++pos;
        return *this;
    }

    ProxyIterator<T>& operator--()
    {
        --pos;
        return *this;
    }

    reference operator[](difference_type n) const { return *(*this + n); }

    ProxyIterator<T>& operator+=(difference_type n)
    {
        pos += n;
        return *this;
    }

    ProxyIterator<T>& operator-=(difference_type n)
    {
        pos -= n;
        return *this;
    }

    friend ProxyIterator<T>
    operator+(const ProxyIterator<T>& lhs, difference_type rhs)
    {
        return ProxyIterator<T>(*lhs.collection, lhs.pos + rhs);
    }

    friend ProxyIterator<T>
    operator+(difference_type lhs, const ProxyIterator<T>& rhs)
    {
        return ProxyIterator<T>(*rhs.collection, rhs.pos + lhs);
    }

    friend ProxyIterator<T>
    operator-(const ProxyIterator<T>& lhs, difference_type rhs)
    {
        return ProxyIterator<T>(*lhs.collection, lhs.pos - rhs);
    }

    friend difference_type
    operator-(const ProxyIterator<T>& lhs, const ProxyIterator<T>& rhs)
    {
        return lhs.pos - rhs.pos;
    }

    friend bool
    operator==(const ProxyIterator<T>& lhs, const ProxyIterator<T>& rhs)
    {
        assert(lhs.collection == rhs.collection);
        return lhs.pos == rhs.pos;
    }

    friend auto
    operator<=>(const ProxyIterator<T>& lhs, const ProxyIterator<T>& rhs)
    {
        assert(lhs.collection == rhs.collection);
        return lhs.pos <=> rhs.pos;
    }
};

template <typename T>
class ProxyCollection : public std::ranges::view_interface<ProxyCollection<T>> {
public:
    auto begin() const
    {
        static_assert(std::random_access_iterator<ProxyIterator<T>>);
        return ProxyIterator<T>(*static_cast<const T*>(this), 0);
    }

    auto end() const
    {
        static_assert(std::random_access_iterator<ProxyIterator<T>>);
        return ProxyIterator<T>(*static_cast<const T*>(this), size());
    }

    std::size_t size() const { return static_cast<const T*>(this)->size(); }
};

/**
 * @brief The VariableProxy class represents a single variable
 * \f$v \in \varspaceof{\task}\f$ of a planning task \f$\task\f$.
 *
 * @ingroup variables
 */
class VariableProxy {
    friend class VariablesProxy;
    friend class FactProxy;

    const AbstractPlanningTask* task;
    int id;

    /// Constructs a variable proxy from the given task and a variable index.
    VariableProxy(const AbstractPlanningTask& task, int var_id);

public:
    VariableProxy(const VariableProxy&) = default;
    VariableProxy(VariableProxy&&) = default;

    VariableProxy& operator=(const VariableProxy&) = delete;
    VariableProxy& operator=(VariableProxy&&) = delete;

    /// Get the variable ID of the variable, i.e. its index in
    /// \f$\{0, ..., |\varspaceof{\task}| - 1]\}\f$.
    int get_id() const;

    /// Get the human-readable name associated with the variable.
    std::string get_name() const;

    /// Get the size \f$|D(v)|\f$ of this variable's domain.
    int get_domain_size() const;

    /// Get a fact for a domain value
    /// \f$d \in \{0, ..., |\domainof{\task}(v)| - 1\}\f$ for this variable.
    FactProxy get_fact(int value) const;

    // Checks if two variables proxies refer to the same variable.
    friend bool
    operator==(const VariableProxy& left, const VariableProxy& right);
};

/**
 * @brief The FactProxy class represents a single fact
 * \f$f \in \factsof{\task}\f$ of a planning task \f$\task\f$.
 *
 * @ingroup facts
 */
class FactProxy {
    friend class VariableProxy;
    friend class OperatorPreconditionProxy;
    friend class OperatorEffectProxy;
    friend class GoalProxy;
    friend class FactsProxy;
    friend class StateFactsProxy;

    friend class probfd::ProbabilisticEffectProxy;

    const AbstractPlanningTask* task;
    FactPair fact;

    /// Construct the proxy for a given task and a fact of this task given by
    /// variable index and domain value.
    FactProxy(const AbstractPlanningTask& task, int var_id, int value);

    /// Construct the proxy for a given task and a fact of this task given by
    /// a FactPair.
    FactProxy(const AbstractPlanningTask& task, const FactPair& fact);

public:
    FactProxy(const FactProxy&) = default;
    FactProxy(FactProxy&&) = default;

    FactProxy& operator=(const FactProxy&) = delete;
    FactProxy& operator=(FactProxy&&) = delete;

    /// Get a proxy to the variable of this fact.
    VariableProxy get_variable() const;

    /// Get the value for the variable of this fact.
    int get_value() const;

    /// Get the FactPair of this fact.
    FactPair get_pair() const;

    /// Get the name of this fact.
    std::string get_name() const;

    /// Checks if two proxies refer to the same fact.
    friend bool operator==(const FactProxy& left, const FactProxy& right);
};

/*
 * @brief The AbstractOperatorProxy class represents a abstract operator
 * \f$a \in \operatorsof{\task}\f$ of a planning task \f$\task\f$.
 *
 * @ingroup operators
 *
 * @note This class models an abstract operator.
 * An abstract operator does not have (stochastic) effects or costs, since
 * these concepts differ between classical and probabilistic planning task..
 */
class AbstractOperatorProxy {
    friend class AbstractOperatorsProxy;

protected:
    const AbstractPlanningTask* task;
    int index;

    /// Constructs the proxy object for the given task and task operator index.
    AbstractOperatorProxy(const AbstractPlanningTask& task, int index);

    /// Constructs the proxy object for the given task and task operator index.
    AbstractOperatorProxy(const AbstractPlanningTask& task, OperatorID op_id);

public:
    AbstractOperatorProxy(const AbstractOperatorProxy&) = default;
    AbstractOperatorProxy(AbstractOperatorProxy&&) = default;

    AbstractOperatorProxy& operator=(const AbstractOperatorProxy&) = delete;
    AbstractOperatorProxy& operator=(AbstractOperatorProxy&&) = delete;

    /// Get the precondition \f$\precondition_a\f$ of the operator.
    OperatorPreconditionProxy get_preconditions() const;

    /// Get the human-readable name associated with the operator.
    std::string get_name() const;

    /// Get the operator's index in the set
    /// \f$\{ 0, ..., |\operatorsof{\task}| - 1 \}\f$.
    int get_id() const;

    // Checks if two proxies represent the same operator.
    friend bool operator==(
        const AbstractOperatorProxy& left,
        const AbstractOperatorProxy& right);
};

/**
 * @brief The OperatorProxy class represents an operator
 * \f$a \in \operatorsof{\task}\f$ of a classical planning task.
 *
 * @ingroup operators
 */
class OperatorProxy : public AbstractOperatorProxy {
    friend class OperatorsProxy;

    // Constructs the proxy object for the given task and task operator index.
    OperatorProxy(const ClassicalPlanningTask& task, int index);

    // Constructs the proxy object for the given task and task operator index.
    OperatorProxy(const ClassicalPlanningTask& task, OperatorID op_id);

public:
    OperatorProxy(const OperatorProxy&) = default;
    OperatorProxy(OperatorProxy&&) = default;

    OperatorProxy& operator=(const OperatorProxy&) = delete;
    OperatorProxy& operator=(OperatorProxy&&) = delete;

    /// Get the effect \f$\eff_a\f$ of the operator.
    OperatorEffectProxy get_effects() const;

    /// Get the integer cost \f$\cost(a)\f$ of the operator.
    int get_cost() const;
};

/**
 * @brief The OperatorPreconditionProxy class represents the precondition
 * \f$\precondition_a\f$ of an operator \f$a \in \operatorsof{\task}\f$ of a
 * classical planning task \f$\task\f$.
 *
 * This class can be used like a range of precondition facts (FactProxy):
 * ```
 * OperatorProxy op = ...;
 *
 * for (const FactProxy& precondition_fact : op.get_preconditions()) {
 *     // Do something with the precondition fact...
 * }
 * ```
 * This range is always sorted by the lexicographic fact ordering.
 *
 * @ingroup operators
 */
class OperatorPreconditionProxy
    : public ProxyCollection<OperatorPreconditionProxy> {
    friend class AbstractOperatorProxy;
    friend class OperatorProxy;

    const AbstractPlanningTask* task;
    int op_index;

    // Constructs the proxy object for the given task and operator given by
    // index.
    OperatorPreconditionProxy(const AbstractPlanningTask& task, int op_index);

public:
    OperatorPreconditionProxy(const OperatorPreconditionProxy&) = default;
    OperatorPreconditionProxy(OperatorPreconditionProxy&&) = default;

    OperatorPreconditionProxy&
    operator=(const OperatorPreconditionProxy&) = delete;
    OperatorPreconditionProxy& operator=(OperatorPreconditionProxy&&) = delete;

    /// Get the number of precondition facts \f$|\precondition_a|\f$ of the
    /// operator.
    std::size_t size() const;

    /// Get a precondition fact by its index in
    /// \f$\{0, ..., |\precondition_a| - 1\}\f$
    FactProxy operator[](std::size_t fact_index) const;

    auto to_pairs() const
    {
        return *this | std::views::transform(&FactProxy::get_pair);
    }
};

/**
 * @brief The OperatorEffectProxy class represents the effect
 * \f$\eff_a\f$ of an deterministic operator \f$a \in \operatorsof{\task}\f$
 * of a classical planning task \f$\task\f$.
 *
 * This class can be used like a range of effect facts (FactProxy). Example:
 * ```
 * OperatorProxy op = ...;
 *
 * for (const FactProxy& effect_fact : op.get_effects()) {
 *     // Do something with the effect fact...
 * }
 * ```
 * This range is always sorted by the lexicographic fact ordering.
 *
 * @ingroup operators
 */
class OperatorEffectProxy : public ProxyCollection<OperatorEffectProxy> {
    friend class OperatorProxy;

    const ClassicalPlanningTask* task;
    int op_index;

    // Constructs the proxy object for the given task and task operator index.
    OperatorEffectProxy(const ClassicalPlanningTask& task, int op_index);

public:
    OperatorEffectProxy(const OperatorEffectProxy&) = default;
    OperatorEffectProxy(OperatorEffectProxy&&) = default;

    OperatorEffectProxy& operator=(const OperatorEffectProxy&) = delete;
    OperatorEffectProxy& operator=(OperatorEffectProxy&&) = delete;

    /// Get the number of effect facts \f$|\eff_a|\f$ of the operator.
    std::size_t size() const;

    /// Get an effect fact by its index in \f$\{0, ..., |\eff_a| - 1\}\f$.
    FactProxy operator[](std::size_t eff_index) const;

    auto to_pairs() const
    {
        return *this | std::views::transform(&FactProxy::get_pair);
    }
};

/**
 * @brief The GoalProxy class represents the goal \f$\goalof{\task}\f$ of an
 * abstract planning task \f$\task\f$.
 *
 * This class can be used like a range of goal facts (FactProxy). Example:
 * ```
 * GoalProxy goal = task.get_goal();
 *
 * for (const FactProxy& goal_fact : goal) {
 *     // Do something with the goal fact...
 * }
 * ```
 * This range is always sorted by the lexicographic fact ordering.
 *
 * @ingroup planning_tasks
 */
class GoalProxy : public ProxyCollection<GoalProxy> {
    friend class AbstractPlanningTask;

    const AbstractPlanningTask* task;

    // Construct a proxy object representing this task's goal.
    explicit GoalProxy(const AbstractPlanningTask& task);

public:
    GoalProxy(const GoalProxy&) = default;
    GoalProxy(GoalProxy&&) = default;

    GoalProxy& operator=(const GoalProxy&) = delete;
    GoalProxy& operator=(GoalProxy&&) = delete;

    /// Get the number of goal facts \f$|\taskgoalof{\task}|\f$.
    std::size_t size() const;

    /// Get a goal fact by its index in
    /// \f$\{ 0, ..., |\taskgoalof{\task}| - 1 \}\f$.
    FactProxy operator[](std::size_t index) const;

    auto to_pairs() const
    {
        return *this | std::views::transform(&FactProxy::get_pair);
    }
};

/**
 * @brief The FactsProxy class represents the set of facts
 * \f$\factsof{\task} = \{ (v, d) \mid v \in \varspaceof{\task},
 * d \in \domainof{\task}(v) \}\f$
 * of a planning task \f$\task\f$.
 *
 * This class can be used like a range of facts (FactProxy). Example:
 * ```
 * FactsProxy all_facts(task);
 *
 * for (const FactProxy& fact : all_facts) {
 *     // Do something with the fact...
 * }
 * ```
 *
 * The facts are ordered lexicographically, primarily by increasing variable ID,
 * and secondarily in order of increasing variable value ID.
 *
 * @ingroup facts
 */
class FactsProxy {
    friend class AbstractPlanningTask;

    const AbstractPlanningTask* task;
    mutable int num_facts = -1;

public:
    class FactsProxyIterator {
        const AbstractPlanningTask* task;
        int var_id;
        int value;

    public:
        using difference_type = int;
        using value_type = FactProxy;
        using reference = FactProxy&;
        using iterator_category = std::forward_iterator_tag;

        FactsProxyIterator() = default;

        FactsProxyIterator(
            const AbstractPlanningTask& task,
            int var_id,
            int value);

        FactProxy operator*() const;

        FactsProxyIterator& operator++();
        FactsProxyIterator operator++(int);

        friend bool operator==(
            const FactsProxyIterator& left,
            const FactsProxyIterator& right);
    };

    // Construct a proxy for the facts of this planning task.
    explicit FactsProxy(const AbstractPlanningTask& task);

public:
    FactsProxy(const FactsProxy&) = default;
    FactsProxy(FactsProxy&&) = default;

    FactsProxy& operator=(const FactsProxy&) = delete;
    FactsProxy& operator=(FactsProxy&&) = delete;

    /// Returns an input iterator to the beginning of the fact range.
    FactsProxyIterator begin() const;

    /// Returns an input iterator to the end of the fact range.
    FactsProxyIterator end() const;

    /// Returns the total number of facts.
    int size() const;

    auto to_pairs() const
    {
        return *this | std::views::transform(&FactProxy::get_pair);
    }
};

/**
 * @brief The FactsProxy class represents the set of facts
 * \f$\{ (v, s(v)) \mid v \in \varspace \}\f$
 * induced by a state \f$s\f$.
 *
 * This class can be used like a range of facts (FactProxy). Example:
 * ```
 * StateFactsProxy state_facts = task.get_state_facts(state);
 *
 * for (const FactProxy& fact : state_facts) {
 *     // Do something with the fact...
 * }
 * ```
 *
 * The facts are ordered by increasing variable ID.
 * Note that every variable is mapped to exactly one value.
 *
 * @ingroup states
 */
class StateFactsProxy : public ProxyCollection<StateFactsProxy> {
    friend class AbstractPlanningTask;

    const AbstractPlanningTask* task;
    const State* state;

    // Construct a proxy for the facts induced by this state.
    StateFactsProxy(const AbstractPlanningTask& task, const State& state);

public:
    StateFactsProxy(const StateFactsProxy&) = default;
    StateFactsProxy(StateFactsProxy&&) = default;

    StateFactsProxy& operator=(const StateFactsProxy&) = delete;
    StateFactsProxy& operator=(StateFactsProxy&&) = delete;

    /// Get the state fact corresponding to a variable identified by its index
    /// in \f$\{ 0, ..., |\varspaceof{\task}| - 1\}\f$.
    FactProxy operator[](std::size_t index) const;

    /// Get the state fact corresponding to a variable.
    FactProxy operator[](VariableProxy variable) const;

    /// Returns the total number of facts of this state, i.e., the number of
    /// variables.
    std::size_t size() const;

    auto to_pairs() const
    {
        return *this | std::views::transform(&FactProxy::get_pair);
    }
};

/**
 * @brief The VariablesProxy class represents the set of variables
 * \f$\varspaceof{\task}\f$ of a planning task \f$\task\f$.
 *
 * This class can be used like a range of variables (VariableProxy):
 * ```
 * VariablesProxy variables = task.get_variables();
 *
 * for (const VariableProxy& var : variables) {
 *     // Do something with the variable...
 * }
 * ```
 *
 * @ingroup variables
 */
class VariablesProxy : public ProxyCollection<VariablesProxy> {
    friend class AbstractPlanningTask;

    const AbstractPlanningTask* task;

    // Constructs the proxy object for the variables of this task.
    explicit VariablesProxy(const AbstractPlanningTask& task);

public:
    VariablesProxy(const VariablesProxy&) = default;
    VariablesProxy(VariablesProxy&&) = default;

    VariablesProxy& operator=(const VariablesProxy&) = delete;
    VariablesProxy& operator=(VariablesProxy&&) = delete;

    /// Get the number of variables \f$|\varspaceof{\task}|\f$ of the task.
    std::size_t size() const;

    /// Get a variable by its index in
    /// \f$\{ 0, ..., |\varspaceof{\task}| - 1\}\f$.
    VariableProxy operator[](std::size_t index) const;

    /// Get the set of all facts \f$\factsof{\task}\f$ of the task.
    FactsProxy get_facts() const;
};

/**
 * @brief The OperatorsProxy class represents the set of operators
 * \f$\operatorsof{\task}\f$ of a classical planning task \f$\task\f$.
 *
 * @ingroup operators
 *
 * This class can be used like a range of operators (OperatorProxy):
 * ```
 * OperatorsProxy operators = task.get_operators();
 *
 * for (const OperatorProxy& op : operators) {
 *     // Do something with the operator...
 * }
 * ```
 * The operators are not ordered in any particular way, but the order always
 * remains the same.
 */
class OperatorsProxy : public ProxyCollection<OperatorsProxy> {
    friend class ClassicalPlanningTask;

    const ClassicalPlanningTask* task;

    // Construct a proxy object representing this task's operators.
    explicit OperatorsProxy(const ClassicalPlanningTask& task);

public:
    OperatorsProxy(const OperatorsProxy&) = default;
    OperatorsProxy(OperatorsProxy&&) = default;

    OperatorsProxy& operator=(const OperatorsProxy&) = delete;
    OperatorsProxy& operator=(OperatorsProxy&&) = delete;

    /// Get the number of operators \f$|\operatorsof{\task}|\f$.
    std::size_t size() const;

    /// Get an operator by its index in
    /// \f$\{ 0, ..., |\operatorsof{\task}| - 1\}\f$.
    OperatorProxy operator[](std::size_t index) const;

    /// Get an operator by its OperatorID, representing an index in
    /// \f$\{ 0, ..., |\operatorsof{\task}| - 1\}\f$.
    OperatorProxy operator[](OperatorID id) const;
};

/*
 * @brief The AbstractOperatorsProxy class represents the set of abstract
 * operators \f$\operatorsof{\task}\f$ of an abstract planning task.
 *
 * @ingroup operators
 */
class AbstractOperatorsProxy : public ProxyCollection<AbstractOperatorsProxy> {
    friend class AbstractPlanningTask;

    const AbstractPlanningTask* task;

    // Construct a proxy object representing this task's abstract operators.
    explicit AbstractOperatorsProxy(const AbstractPlanningTask& task);

public:
    AbstractOperatorsProxy(const AbstractOperatorsProxy&) = default;
    AbstractOperatorsProxy(AbstractOperatorsProxy&&) = default;

    AbstractOperatorsProxy& operator=(const AbstractOperatorsProxy&) = delete;
    AbstractOperatorsProxy& operator=(AbstractOperatorsProxy&&) = delete;

    /// Get the number of abstract operators \f$|\operatorsof{\task}|\f$.
    std::size_t size() const;

    /// Get an abstract operator by its index in
    /// \f$\{ 0, ..., |\operatorsof{\task}| - 1\}\f$.
    AbstractOperatorProxy operator[](std::size_t index) const;

    /// Get an abstract operator by its OperatorID, representing an index in
    /// \f$\{ 0, ..., |\operatorsof{\task}| - 1\}\f$.
    AbstractOperatorProxy operator[](OperatorID id) const;
};

/// Prints an input range of FactProxy values to an output stream.
std::ostream&
operator<<(std::ostream& out, const InputRange<FactProxy> auto& facts)
{
    out << "{ ";

    if (!facts.empty()) {
        out << facts.front().get_name();

        for (const auto& fact_proxy : facts | std::views::drop(1)) {
            out << ", " << fact_proxy.get_name();
        }
    }

    return out << " }";
}

#endif // DOWNWARD_TASK_PROXY_H
