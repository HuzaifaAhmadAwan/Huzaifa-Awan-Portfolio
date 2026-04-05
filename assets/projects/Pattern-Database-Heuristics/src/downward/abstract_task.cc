#include "downward/abstract_task.h"
#include "downward/state.h"
#include "downward/task_proxy.h"

#include "downward/plugins/plugin.h"

#include <iostream>

using namespace std;

VariablesProxy AbstractPlanningTask::get_variables() const
{
    return VariablesProxy(*this);
}

FactsProxy AbstractPlanningTask::get_facts() const
{
    return FactsProxy(*this);
}

AbstractOperatorsProxy AbstractPlanningTask::get_abstract_operators() const
{
    return AbstractOperatorsProxy(*this);
}

GoalProxy AbstractPlanningTask::get_goal() const
{
    return GoalProxy(*this);
}

StateFactsProxy
AbstractPlanningTask::get_state_facts(const State& state) const
{
    return StateFactsProxy(*this, state);
}

State AbstractPlanningTask::create_state(
    std::vector<int>&& state_values) const
{
    return State(std::move(state_values));
}

State AbstractPlanningTask::create_state(
    const StateRegistry& registry,
    StateID id,
    const PackedStateBin* buffer) const
{
    return State(registry, id, buffer);
}

State AbstractPlanningTask::get_initial_state() const
{
    return create_state(this->get_initial_state_values());
}

void AbstractPlanningTask::subscribe_to_task_destruction(
    subscriber::Subscriber<AbstractPlanningTask>* subscriber) const
{
    this->subscribe(subscriber);
}

TaskID AbstractPlanningTask::get_id() const
{
    return TaskID(this);
}

static class AbstractTaskCategoryPlugin
    : public plugins::TypedCategoryPlugin<ClassicalPlanningTask> {
public:
    AbstractTaskCategoryPlugin()
        : TypedCategoryPlugin("ClassicalPlanningTask")
    {
        // TODO: Replace empty string by synopsis for the wiki page.
        document_synopsis("");
    }
} _category_plugin;
