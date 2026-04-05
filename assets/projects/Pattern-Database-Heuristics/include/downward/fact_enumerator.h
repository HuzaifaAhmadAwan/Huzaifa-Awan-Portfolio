#ifndef FACT_ENUMERATOR_H
#define FACT_ENUMERATOR_H

#include <vector>

class VariablesProxy;
class FactProxy;
struct FactPair;

/**
 * @brief Maps the facts of a task to indices in \f$\{ 0, \dots, |F| - 1 \}\f$,
 * where \f$F\f$ is the set of all facts.
 *
 * In detail, let \f$\vars = \{ v_0, \dots, v_n \}\f$ be the set of variables
 * ordered by index, i.e, the index of \f$v_i\f$ is \f$i\f$.
 * Then the fact \f$v_k=d\f$ is mapped to the index
 * \f$\sum_{i = 0}^{k - 1} |\domain(v_i)| + d\f$, where
 * \f$d \in \{ 0, \dots, |\domain(v_k)| - 1\}\f$ is some domain value of
 * \f$v_i\f$.
 *
 * This indexing method provides a lexicographic ordering of facts, primarily by
 * increasing variable index and secondarily by increasing domain value.
 * Initial construction takes time linear in the number of facts, but afterwards
 * querying a fact index takes constant time.
 * In contrast, the following code would compute the same indices, but takes
 * time linear in the number of variables on *every* invocation, which quickly
 * accumulates a runtime overhead as time progresses.
 * ```
 * int compute_fact_index(const FactPair& fact) {
 *     int fact_index = 0;
 *     for (const VariableProxy& variable : task.get_variables()) {
 *         if (variable.get_id() == fact.var) {
 *              return fact_index + fact.value;
 *          }
 *
 *         fact_index += variable.get_domain_size();
 *     }
 * }
 * ```
 *
 * This class is useful to map facts to indices in a one-dimensional array.
 * Idea:
 * ```
 * FactEnumerator enumerator(task_proxy.get_variables());
 *
 * // Array storing information for each fact (index).
 * std::vector<int> fact_table(enumerator.get_num_facts(), 0);
 *
 * // Store information for a fact
 * FactPair fact = ...;
 * int table_index = enumerator.get_fact_index(fact);
 * fact_table[table_index] = 42;
 *
 * // Retreive information for a fact
 * FactPair other_fact = ...;
 * int other_table_index = enumerator.get_fact_index(other_fact);
 * int fact_value = fact_table[other_table_index];
 * ```
 *
 * @ingroup downward_other
 */
class FactEnumerator {
    std::vector<int> indices;
    int num_facts;

public:
    /// Constructs the enumerator object from the set of all variables.
    explicit FactEnumerator(const VariablesProxy& variables);

    /// Returns the index of the given fact.
    int get_fact_index(int var, int value) const;

    /// Returns the index of the given fact.
    int get_fact_index(const FactPair& fact) const;

    /// Returns the index of the given fact.
    int get_fact_index(const FactProxy& fact) const;

    /// Returns the total number of facts \f$F\f$ of the planning task passed on
    /// construction.
    int get_num_facts() const;
};

#endif
