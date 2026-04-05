#include "downward/heuristics/goal_count_heuristic.h"

#include "downward/heuristic.h"

#include "downward/state.h"
#include "downward/task_proxy.h"

#include "downward/plugins/plugin.h"

using namespace std;

namespace goal_count_heuristic {

class GoalCountHeuristic : public Heuristic {
public:
    explicit GoalCountHeuristic(
        const std::shared_ptr<ClassicalPlanningTask>& task);

    int compute_heuristic(const State& state) override;
};

GoalCountHeuristic::GoalCountHeuristic(
    const shared_ptr<ClassicalPlanningTask>& task)
    : Heuristic(task)
{
}

int GoalCountHeuristic::compute_heuristic(const State& state)
{
    int unachieved_goals = 0;
    for (FactProxy goal : task->get_goal()) {
        if (state[goal.get_variable()] != goal.get_value()) {
            ++unachieved_goals;
        }
    }
    return unachieved_goals;
}

std::unique_ptr<Heuristic>
create_goal_count_heuristic(std::shared_ptr<ClassicalPlanningTask> task)
{
    return std::make_unique<goal_count_heuristic::GoalCountHeuristic>(
        std::move(task));
}

class GoalCountHeuristicFeature
    : public plugins::TypedFeature<Evaluator, Heuristic> {
public:
    GoalCountHeuristicFeature()
        : TypedFeature("goalcount")
    {
        document_title("Goal count heuristic");

        add_heuristic_options_to_feature(*this, "goalcount");

        document_language_support("action costs", "ignored by design");
        document_language_support("conditional effects", "supported");
        document_language_support("axioms", "supported");

        document_property("admissible", "no");
        document_property("consistent", "no");
        document_property("safe", "yes");
        document_property("preferred operators", "no");
    }

    virtual shared_ptr<Heuristic>
    create_component(const plugins::Options& opts, const utils::Context&)
        const override
    {
        return create_goal_count_heuristic(
            std::get<0>(get_heuristic_arguments_from_options(opts)));
    }
};

static plugins::FeaturePlugin<GoalCountHeuristicFeature> _plugin;
} // namespace goal_count_heuristic
