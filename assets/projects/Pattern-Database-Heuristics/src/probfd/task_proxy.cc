#include "probfd/task_proxy.h"

#include "downward/utils/collections.h"

#include <cassert>
#include <vector>

namespace probfd {

ProbabilisticEffectProxy::ProbabilisticEffectProxy(
    const ProbabilisticPlanningTask& task,
    int op_index,
    int outcome_index)
    : task_(&task)
    , op_index_(op_index)
    , outcome_index_(outcome_index)
{
}

std::size_t ProbabilisticEffectProxy::size() const
{
    return task_->get_num_operator_outcome_effects(op_index_, outcome_index_);
}

FactProxy ProbabilisticEffectProxy::operator[](std::size_t eff_index) const
{
    assert(eff_index < size());
    return FactProxy(
        *task_,
        task_->get_operator_outcome_effect(
            op_index_,
            outcome_index_,
            eff_index));
}

ProbabilisticOutcomeProxy::ProbabilisticOutcomeProxy(
    const ProbabilisticPlanningTask& task,
    int op_index,
    int outcome_index)
    : task_(&task)
    , op_index_(op_index)
    , outcome_index_(outcome_index)
{
}

ProbabilisticOperatorProxy ProbabilisticOutcomeProxy::get_operator() const
{
    return ProbabilisticOperatorProxy(*task_, op_index_);
}

ProbabilisticEffectProxy ProbabilisticOutcomeProxy::get_effects() const
{
    return ProbabilisticEffectProxy(*task_, op_index_, outcome_index_);
}

value_t ProbabilisticOutcomeProxy::get_probability() const
{
    return task_->get_operator_outcome_probability(op_index_, outcome_index_);
}

ProbabilisticOutcomesProxy::ProbabilisticOutcomesProxy(
    const ProbabilisticPlanningTask& task,
    int op_index)
    : task_(&task)
    , op_index_(op_index)
{
}

std::size_t ProbabilisticOutcomesProxy::size() const
{
    return task_->get_num_operator_outcomes(op_index_);
}

ProbabilisticOperatorProxy ProbabilisticOutcomesProxy::get_operator() const
{
    return ProbabilisticOperatorProxy(*task_, op_index_);
}

ProbabilisticOutcomeProxy
ProbabilisticOutcomesProxy::operator[](std::size_t eff_index) const
{
    assert(eff_index < size());
    return ProbabilisticOutcomeProxy(*task_, op_index_, eff_index);
}

ProbabilisticOperatorProxy::ProbabilisticOperatorProxy(
    const ProbabilisticPlanningTask& task,
    int index)
    : AbstractOperatorProxy(task, index)
{
}

ProbabilisticOutcomesProxy ProbabilisticOperatorProxy::get_outcomes() const
{
    return ProbabilisticOutcomesProxy(
        *static_cast<const ProbabilisticPlanningTask*>(task),
        index);
}

value_t ProbabilisticOperatorProxy::get_cost() const
{
    return static_cast<const ProbabilisticPlanningTask*>(task)
        ->get_operator_cost(index);
}

ProbabilisticOperatorsProxy::ProbabilisticOperatorsProxy(
    const ProbabilisticPlanningTask& task)
    : task_(&task)
{
}

std::size_t ProbabilisticOperatorsProxy::size() const
{
    return task_->get_num_operators();
}

ProbabilisticOperatorProxy
ProbabilisticOperatorsProxy::operator[](std::size_t index) const
{
    assert(index < size());
    return ProbabilisticOperatorProxy(*task_, index);
}

ProbabilisticOperatorProxy
ProbabilisticOperatorsProxy::operator[](OperatorID id) const
{
    return (*this)[id.get_index()];
}

ProbabilisticOperatorsProxy ProbabilisticPlanningTask::get_operators() const
{
    return ProbabilisticOperatorsProxy(*this);
}

} // namespace probfd
