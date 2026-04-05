#include "downward/evaluator.h"

#include "downward/plugins/plugin.h"
#include "downward/utils/logging.h"
#include "downward/utils/system.h"

#include "downward/evaluation_context.h"

#include <cassert>

using namespace std;

bool Evaluator::dead_ends_are_reliable() const
{
    return true;
}

vector<EvaluationResult>
Evaluator::compute_results(vector<EvaluationContext>& eval_contexts)
{
    vector<EvaluationResult> eval_results;
    for (auto& ec : eval_contexts) {
        eval_results.push_back(compute_result(ec));
    }
    return eval_results;
}

static class EvaluatorCategoryPlugin
    : public plugins::TypedCategoryPlugin<Evaluator> {
public:
    EvaluatorCategoryPlugin()
        : TypedCategoryPlugin("Evaluator")
    {
        document_synopsis(
            "An evaluator specification is either a newly created evaluator "
            "instance or an evaluator that has been defined previously. "
            "This page describes how one can specify a new evaluator instance. "
            "For re-using evaluators, see "
            "OptionSyntax#Evaluator_Predefinitions.\n\n"
            "If the evaluator is a heuristic, "
            "definitions of //properties// in the descriptions below:\n\n"
            " * **admissible:** h(s) <= h*(s) for all states s\n"
            " * **consistent:** h(s) <= c(s, s') + h(s') for all states s "
            "connected to states s' by an action with cost c(s, s')\n"
            " * **safe:** h(s) = infinity is only true for states "
            "with h*(s) = infinity\n"
            " * **preferred operators:** this heuristic identifies "
            "preferred operators ");
        allow_variable_binding();
    }
} _category_plugin;
