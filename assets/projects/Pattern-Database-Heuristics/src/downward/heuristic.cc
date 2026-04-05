#include "downward/heuristic.h"

#include "downward/evaluation_context.h"
#include "downward/evaluation_result.h"

#include "downward/plugins/plugin.h"
#include "downward/tasks/root_task.h"

#include <cassert>

using namespace std;

Heuristic::Heuristic(const shared_ptr<ClassicalPlanningTask>& transform)
    : EvaluatorBase(true, true, true, "", utils::Verbosity::SILENT)
    , task(transform)
{
}

Heuristic::~Heuristic() = default;

void add_heuristic_options_to_feature(
    plugins::Feature& feature,
    const string&)
{
    feature.add_option<shared_ptr<ClassicalPlanningTask>>(
        "transform",
        "Optional task transformation for the heuristic."
        " Currently, adapt_costs() and no_transform() are available.",
        "no_transform()");
}

tuple<shared_ptr<ClassicalPlanningTask>>
get_heuristic_arguments_from_options(const plugins::Options& opts)
{
    return tuple_cat(
        make_tuple(
            opts.get<shared_ptr<ClassicalPlanningTask>>("transform")));
}

EvaluationResult Heuristic::compute_result(EvaluationContext& eval_context)
{
    EvaluationResult result;

    const State& state = eval_context.get_state();

    int heuristic = compute_heuristic(state);
    result.set_count_evaluation(true);

    assert(heuristic == DEAD_END || heuristic >= 0);

    if (heuristic == DEAD_END) {
        /*
          It is permissible to mark preferred operators for dead-end
          states (thus allowing a heuristic to mark them on-the-fly
          before knowing the final result), but if it turns out we
          have a dead end, we don't want to actually report any
          preferred operators.
        */
        heuristic = EvaluationResult::INFTY;
    }

    result.set_evaluator_value(heuristic);

    return result;
}

static class HeuristicCategoryPlugin
    : public plugins::TypedCategoryPlugin<Heuristic> {
public:
    HeuristicCategoryPlugin()
        : TypedCategoryPlugin("Heuristic")
    {
    }
} _category_plugin;