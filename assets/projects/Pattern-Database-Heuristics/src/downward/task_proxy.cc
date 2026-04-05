#include "downward/task_proxy.h"

#include "downward/state_registry.h"

#include "downward/task_utils/causal_graph.h"

#include "downward/utils/collections.h"
#include "downward/utils/hash.h"

#include <cassert>
#include <numeric>

using namespace std;

FactProxy::FactProxy(const AbstractPlanningTask& task, const FactPair& fact)
    : task(&task)
    , fact(fact)
{
    assert(fact.var >= 0 && fact.var < task.get_num_variables());
    assert(fact.value >= 0 && fact.value < get_variable().get_domain_size());
}

FactProxy::FactProxy(const AbstractPlanningTask& task, int var_id, int value)
    : FactProxy(task, FactPair(var_id, value))
{
}

VariableProxy FactProxy::get_variable() const
{
    return VariableProxy(*task, fact.var);
}

int FactProxy::get_value() const
{
    return fact.value;
}

FactPair FactProxy::get_pair() const
{
    return fact;
}

std::string FactProxy::get_name() const
{
    return task->get_fact_name(fact);
}

bool operator==(const FactProxy& left, const FactProxy& right)
{
    assert(left.task == right.task);
    return left.fact == right.fact;
}

FactsProxy::FactsProxyIterator::FactsProxyIterator(
    const AbstractPlanningTask& task,
    int var_id,
    int value)
    : task(&task)
    , var_id(var_id)
    , value(value)
{
}

FactProxy FactsProxy::FactsProxyIterator::operator*() const
{
    return FactProxy(*task, var_id, value);
}

FactsProxy::FactsProxyIterator& FactsProxy::FactsProxyIterator::operator++()
{
    assert(var_id < task->get_num_variables());
    int num_vals = task->get_variable_domain_size(var_id);
    assert(value < num_vals);
    ++value;
    if (value == num_vals) {
        ++var_id;
        value = 0;
    }
    return *this;
}

FactsProxy::FactsProxyIterator FactsProxy::FactsProxyIterator::operator++(int)
{
    auto r = *this;
    ++(*this);
    return r;
}

bool operator==(
    const FactsProxy::FactsProxyIterator& left,
    const FactsProxy::FactsProxyIterator& right)
{
    assert(left.task == right.task);
    return left.var_id == right.var_id && left.value == right.value;
}

FactsProxy::FactsProxy(const AbstractPlanningTask& task)
    : task(&task)
{
}

FactsProxy::FactsProxyIterator FactsProxy::begin() const
{
    return FactsProxyIterator(*task, 0, 0);
}

FactsProxy::FactsProxyIterator FactsProxy::end() const
{
    return FactsProxyIterator(*task, task->get_num_variables(), 0);
}

int FactsProxy::size() const
{
    if (num_facts == -1) {
        num_facts = std::distance(begin(), end());
    }
    return num_facts;
}

StateFactsProxy::StateFactsProxy(
    const AbstractPlanningTask& task,
    const State& state)
    : task(&task)
    , state(&state)
{
}

FactProxy StateFactsProxy::operator[](std::size_t index) const
{
    return FactProxy(*task, index, (*state)[index]);
}

FactProxy StateFactsProxy::operator[](VariableProxy variable) const
{
    return (*this)[variable.get_id()];
}

std::size_t StateFactsProxy::size() const
{
    return state->size();
}

VariableProxy::VariableProxy(const AbstractPlanningTask& task, int var_id)
    : task(&task)
    , id(var_id)
{
}

int VariableProxy::get_id() const
{
    return id;
}

std::string VariableProxy::get_name() const
{
    return task->get_variable_name(id);
}

int VariableProxy::get_domain_size() const
{
    return task->get_variable_domain_size(id);
}

FactProxy VariableProxy::get_fact(int value) const
{
    assert(value < get_domain_size());
    return FactProxy(*task, id, value);
}

bool operator==(const VariableProxy& left, const VariableProxy& right)
{
    assert(left.task == right.task);
    return left.id == right.id;
}

VariablesProxy::VariablesProxy(const AbstractPlanningTask& task)
    : task(&task)
{
}

std::size_t VariablesProxy::size() const
{
    return task->get_num_variables();
}

VariableProxy VariablesProxy::operator[](std::size_t index) const
{
    assert(index < size());
    return VariableProxy(*task, index);
}

FactsProxy VariablesProxy::get_facts() const
{
    return FactsProxy(*task);
}

OperatorPreconditionProxy::OperatorPreconditionProxy(
    const AbstractPlanningTask& task,
    int op_index)
    : task(&task)
    , op_index(op_index)
{
}

std::size_t OperatorPreconditionProxy::size() const
{
    return task->get_num_operator_preconditions(op_index);
}

FactProxy OperatorPreconditionProxy::operator[](std::size_t fact_index) const
{
    assert(fact_index < size());
    return FactProxy(
        *task,
        task->get_operator_precondition(op_index, fact_index));
}

OperatorEffectProxy::OperatorEffectProxy(
    const ClassicalPlanningTask& task,
    int op_index)
    : task(&task)
    , op_index(op_index)
{
}

std::size_t OperatorEffectProxy::size() const
{
    return task->get_num_operator_effects(op_index);
}

FactProxy OperatorEffectProxy::operator[](std::size_t eff_index) const
{
    assert(eff_index < size());
    return FactProxy(*task, task->get_operator_effect(op_index, eff_index));
}

AbstractOperatorProxy::AbstractOperatorProxy(
    const AbstractPlanningTask& task,
    int index)
    : task(&task)
    , index(index)
{
}

AbstractOperatorProxy::AbstractOperatorProxy(
    const AbstractPlanningTask& task,
    OperatorID op_id)
    : task(&task)
    , index(op_id.get_index())
{
}

OperatorPreconditionProxy AbstractOperatorProxy::get_preconditions() const
{
    return OperatorPreconditionProxy(*task, index);
}

std::string AbstractOperatorProxy::get_name() const
{
    return task->get_operator_name(index);
}

int AbstractOperatorProxy::get_id() const
{
    return index;
}

bool operator==(
    const AbstractOperatorProxy& left,
    const AbstractOperatorProxy& right)
{
    assert(left.task == right.task);
    return left.index == right.index;
}

OperatorProxy::OperatorProxy(const ClassicalPlanningTask& task, int index)
    : AbstractOperatorProxy(task, index)
{
}

OperatorProxy::OperatorProxy(
    const ClassicalPlanningTask& task,
    OperatorID op_id)
    : OperatorProxy(task, op_id.get_index())
{
}

OperatorEffectProxy OperatorProxy::get_effects() const
{
    return OperatorEffectProxy(
        *static_cast<const ClassicalPlanningTask*>(task),
        index);
}

int OperatorProxy::get_cost() const
{
    return static_cast<const ClassicalPlanningTask*>(task)->get_operator_cost(
        index);
}

OperatorsProxy::OperatorsProxy(const ClassicalPlanningTask& task)
    : task(&task)
{
}

std::size_t OperatorsProxy::size() const
{
    return task->get_num_operators();
}

OperatorProxy OperatorsProxy::operator[](std::size_t index) const
{
    assert(index < size());
    return OperatorProxy(*task, index);
}

OperatorProxy OperatorsProxy::operator[](OperatorID id) const
{
    return (*this)[id.get_index()];
}

AbstractOperatorsProxy::AbstractOperatorsProxy(const AbstractPlanningTask& task)
    : task(&task)
{
}

std::size_t AbstractOperatorsProxy::size() const
{
    return task->get_num_operators();
}

AbstractOperatorProxy
AbstractOperatorsProxy::operator[](std::size_t index) const
{
    assert(index < size());
    return AbstractOperatorProxy(*task, index);
}

AbstractOperatorProxy AbstractOperatorsProxy::operator[](OperatorID id) const
{
    return (*this)[id.get_index()];
}

GoalProxy::GoalProxy(const AbstractPlanningTask& task)
    : task(&task)
{
}

std::size_t GoalProxy::size() const
{
    return task->get_num_goals();
}

FactProxy GoalProxy::operator[](std::size_t index) const
{
    assert(index < size());
    return FactProxy(*task, task->get_goal_fact(index));
}

OperatorsProxy ClassicalPlanningTask::get_operators() const
{
    return OperatorsProxy(*this);
}

const causal_graph::CausalGraph&
ClassicalPlanningTask::get_causal_graph() const
{
    return causal_graph::get_causal_graph(this);
}