#ifndef PROBFD_TASK_PROXY_H
#define PROBFD_TASK_PROXY_H

#include "probfd/probabilistic_task.h"

#include "downward/operator_id.h"
#include "downward/task_proxy.h"

// Forward Declarations
namespace probfd {
class ProbabilisticEffectProxy;
class ProbabilisticOperatorProxy;
class ProbabilisticOperatorsProxy;
class ProbabilisticOutcomeProxy;
class ProbabilisticOutcomesProxy;
} // namespace probfd

namespace probfd {

/// Proxy class used to inspect the list of probabilistic effects of a
/// probabilistic operator. Can be used as a range of
/// ProbabilisticEffectProxies, one for each probabilistic effect.
class ProbabilisticEffectProxy
    : public ProxyCollection<ProbabilisticEffectProxy> {
    const ProbabilisticPlanningTask* task_;
    int op_index_;
    int outcome_index_;

public:
    ProbabilisticEffectProxy(
        const ProbabilisticPlanningTask& task,
        int op_index,
        int outcome_index);

    /// Returns the number of probabilistic effects.
    std::size_t size() const;

    /// Accesses a specific probabilistic effect by its list index.
    FactProxy operator[](std::size_t eff_index) const;
};

/// Proxy class used to inspect a single probabilistic outcome of a
/// probabilistic operator.
class ProbabilisticOutcomeProxy {
    const ProbabilisticPlanningTask* task_;
    int op_index_;
    int outcome_index_;

public:
    ProbabilisticOutcomeProxy(
        const ProbabilisticPlanningTask& task,
        int op_index,
        int outcome_index);

    /// Get this outcome's operator.
    ProbabilisticOperatorProxy get_operator() const;

    /// Get a proxy to the probabilistic effects of this outcome.
    ProbabilisticEffectProxy get_effects() const;

    /// Get the probability of this outcome.
    value_t get_probability() const;
};

/// Proxy class used to inspect the list of probabilistic outcomes of a
/// probabilistic operator. Can be used as a range of
/// ProbabilisticOutcomeProxies, one for each probabilistic outcome.
class ProbabilisticOutcomesProxy
    : public ProxyCollection<ProbabilisticOutcomesProxy> {
    const ProbabilisticPlanningTask* task_;
    int op_index_;

public:
    ProbabilisticOutcomesProxy(const ProbabilisticPlanningTask& task, int op_index);

    /// Returns the number of outcomes of the probabilistic operator.
    std::size_t size() const;

    /// Get the operator of the outcomes.
    ProbabilisticOperatorProxy get_operator() const;

    /// Get a proxy to a specific outcome by its list index.
    ProbabilisticOutcomeProxy operator[](std::size_t eff_index) const;
};

/// Proxy class used to inspect a single probabilistic operator.
class ProbabilisticOperatorProxy : public AbstractOperatorProxy {
public:
    ProbabilisticOperatorProxy(const ProbabilisticPlanningTask& task, int index);

    /// Get a proxy to the outcomes of the operator.
    ProbabilisticOutcomesProxy get_outcomes() const;

    /// Get the cost of the operator.
    value_t get_cost() const;
};

/// Proxy class used to inspect a list of probabilistic operators of a
/// probabilistic task. Can be used as a range of ProbabilisticOperatorProxies,
/// one for each probabilistic operator.
class ProbabilisticOperatorsProxy
    : public ProxyCollection<ProbabilisticOperatorsProxy> {
    const ProbabilisticPlanningTask* task_;

public:
    explicit ProbabilisticOperatorsProxy(const ProbabilisticPlanningTask& task);

    /// Returns the number of probabilistic operators in the list.
    std::size_t size() const;

    /// Get a proxy for a specific probabilistic operator by list index.
    ProbabilisticOperatorProxy operator[](std::size_t index) const;

    /// Get a proxy for a specific probabilistic operator by operator id.
    ProbabilisticOperatorProxy operator[](OperatorID id) const;
};

} // namespace probfd

#endif
