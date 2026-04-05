#include "downward/heuristics/blind_search_heuristic.h"

#include "downward/heuristic.h"

#include "downward/task_utils/task_properties.h"

#include "downward/plugins/plugin.h"

using namespace std;

namespace blind_search_heuristic {

class BlindSearchHeuristic : public Heuristic {
    int min_operator_cost;

public:
    explicit BlindSearchHeuristic(
        const std::shared_ptr<ClassicalPlanningTask>& task);

    int compute_heuristic(const State& state) override;
};

BlindSearchHeuristic::BlindSearchHeuristic(
    const shared_ptr<ClassicalPlanningTask>& task)
    : Heuristic(task)
{
    // Check if there are any operators
    if (task->get_operators().size() == 0) {
        min_operator_cost = DEAD_END;
    } else {
        min_operator_cost = task_properties::get_min_operator_cost(*task);
    }
}

int BlindSearchHeuristic::compute_heuristic(const State& state)
{
    if (task_properties::is_goal_state(*task, state)) {
        return 0;
    }
    return min_operator_cost;
}

std::unique_ptr<Heuristic>
create_blind_heuristic(std::shared_ptr<ClassicalPlanningTask> task)
{
    return std::make_unique<blind_search_heuristic::BlindSearchHeuristic>(
        std::move(task));
}

class BlindSearchHeuristicFeature
    : public plugins::TypedFeature<Evaluator, Heuristic> {
public:
    BlindSearchHeuristicFeature()
        : TypedFeature("blind")
    {
        document_title("Blind heuristic");
        document_synopsis(
            "Returns cost of cheapest action for non-goal states, "
            "0 for goal states");

        add_heuristic_options_to_feature(*this, "blind");

        document_language_support("action costs", "supported");
        document_language_support("conditional effects", "supported");
        document_language_support("axioms", "supported");

        document_property("admissible", "yes");
        document_property("consistent", "yes");
        document_property("safe", "yes");
        document_property("preferred operators", "no");
    }

    virtual shared_ptr<Heuristic>
    create_component(const plugins::Options& opts, const utils::Context&)
        const override
    {
        return create_blind_heuristic(
            std::get<0>(get_heuristic_arguments_from_options(opts)));
    }
};

static plugins::FeaturePlugin<BlindSearchHeuristicFeature> _plugin;
} // namespace blind_search_heuristic
