#include "downward/cached_heuristic.h"

#include "downward/heuristic.h"

#include "downward/plugins/plugin.h"

#include "downward/evaluation_context.h"

#include <cassert>

using namespace std;

CachedHeuristic::CachedHeuristic(std::shared_ptr<Heuristic> child)
    : child(std::move(child))
    // TODO: is true really a good idea here?
    , heuristic_cache(HEntry(NO_VALUE, true))
{
}

bool CachedHeuristic::dead_ends_are_reliable() const
{
    return child->dead_ends_are_reliable();
}

void CachedHeuristic::get_path_dependent_evaluators(std::set<Evaluator*>& evals)
{
    return child->get_path_dependent_evaluators(evals);
}

void CachedHeuristic::notify_initial_state(const State& initial_state)
{
    return child->notify_initial_state(initial_state);
}

void CachedHeuristic::notify_state_transition(
    const State& parent_state,
    OperatorID op_id,
    const State& state)
{
    return child->notify_state_transition(parent_state, op_id, state);
}

EvaluationResult
CachedHeuristic::compute_result(EvaluationContext& eval_context)
{
    EvaluationResult result;

    const State& state = eval_context.get_state();

    int heuristic = NO_VALUE;

    if (heuristic_cache[state].h != NO_VALUE &&
        !heuristic_cache[state].dirty) {
        heuristic = heuristic_cache[state].h;
        result.set_count_evaluation(false);
    } else {
        heuristic = child->compute_heuristic(state);
        heuristic_cache[state] = HEntry(heuristic, false);
        result.set_count_evaluation(true);
    }

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

std::vector<EvaluationResult>
CachedHeuristic::compute_results(std::vector<EvaluationContext>& eval_contexts)
{
    return child->compute_results(eval_contexts);
}

const std::string& CachedHeuristic::get_description() const
{
    return child->get_description();
}

bool CachedHeuristic::is_used_for_reporting_minima() const
{
    return child->is_used_for_reporting_minima();
}

bool CachedHeuristic::is_used_for_boosting() const
{
    return child->is_used_for_boosting();
}

bool CachedHeuristic::is_used_for_counting_evaluations() const
{
    return child->is_used_for_counting_evaluations();
}

void CachedHeuristic::report_value_for_initial_state(
    const EvaluationResult& result) const
{
    return child->report_value_for_initial_state(result);
}

void CachedHeuristic::report_new_minimum_value(
    const EvaluationResult& result) const
{
    return child->report_new_minimum_value(result);
}

bool CachedHeuristic::is_estimate_cached(const State& state) const
{
    return heuristic_cache[state].h != NO_VALUE;
}

int CachedHeuristic::get_cached_estimate(const State& state) const
{
    assert(is_estimate_cached(state));
    return heuristic_cache[state].h;
}

namespace {

class CachedHeuristicCategoryPlugin
    : public plugins::TypedCategoryPlugin<CachedHeuristic> {
public:
    CachedHeuristicCategoryPlugin()
        : TypedCategoryPlugin("CachedHeuristic")
    {
    }
} _category_plugin;

class CachedEvaluatorFeature
    : public plugins::TypedFeature<Evaluator, CachedHeuristic> {
public:
    CachedEvaluatorFeature()
        : TypedFeature("cached")
    {
        add_option<std::shared_ptr<Heuristic>>(
            "child",
            "the child heuristic whose estimates are cached",
            plugins::ArgumentInfo::NO_DEFAULT);
    }

    shared_ptr<CachedHeuristic>
    create_component(const plugins::Options& opts, const utils::Context&)
        const override
    {
        return plugins::make_shared_from_arg_tuples<CachedHeuristic>(
            opts.get<std::shared_ptr<Heuristic>>("child"));
    }
};

plugins::FeaturePlugin<CachedEvaluatorFeature> _plugin;

}