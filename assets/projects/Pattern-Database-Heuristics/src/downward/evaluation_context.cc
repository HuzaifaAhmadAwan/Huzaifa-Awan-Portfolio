#include "downward/evaluation_context.h"

#include "downward/evaluation_result.h"
#include "downward/evaluator.h"
#include "downward/search_statistics.h"

#include <cassert>

using namespace std;

EvaluationContext::EvaluationContext(
    const EvaluatorCache& cache,
    const State& state,
    int g_value,
    SearchStatistics* statistics)
    : cache(cache)
    , state(state)
    , g_value(g_value)
    , statistics(statistics)
{
}

EvaluationContext::EvaluationContext(
    const EvaluationContext& other,
    int g_value,
    SearchStatistics* statistics)
    : EvaluationContext(other.cache, other.state, g_value, statistics)
{
}

EvaluationContext::EvaluationContext(
    const State& state,
    int g_value,
    SearchStatistics* statistics)
    : EvaluationContext(EvaluatorCache(), state, g_value, statistics)
{
}

EvaluationContext::EvaluationContext(
    const State& state,
    SearchStatistics* statistics)
    : EvaluationContext(EvaluatorCache(), state, INVALID, statistics)
{
}

const EvaluationResult& EvaluationContext::get_result(Evaluator* evaluator)
{
    EvaluationResult& result = cache[evaluator];
    if (result.is_uninitialized()) {
        result = evaluator->compute_result(*this);
        if (statistics && evaluator->is_used_for_counting_evaluations() &&
            result.get_count_evaluation()) {
            statistics->inc_evaluations();
        }
    }
    return result;
}

const EvaluatorCache& EvaluationContext::get_cache() const
{
    return cache;
}

const State& EvaluationContext::get_state() const
{
    return state;
}

int EvaluationContext::get_g_value() const
{
    assert(g_value != INVALID);
    return g_value;
}

bool EvaluationContext::is_evaluator_value_infinite(Evaluator* eval)
{
    return get_result(eval).is_infinite();
}

int EvaluationContext::get_evaluator_value(Evaluator* eval)
{
    int h = get_result(eval).get_evaluator_value();
    assert(h != EvaluationResult::INFTY);
    return h;
}

int EvaluationContext::get_evaluator_value_or_infinity(Evaluator* eval)
{
    return get_result(eval).get_evaluator_value();
}
