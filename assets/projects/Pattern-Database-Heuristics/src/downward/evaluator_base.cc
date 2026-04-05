#include "downward/evaluator_base.h"

#include "downward/plugins/plugin.h"
#include "downward/utils/logging.h"

#include "downward/evaluation_context.h"

using namespace std;

EvaluatorBase::EvaluatorBase(
    bool use_for_reporting_minima,
    bool use_for_boosting,
    bool use_for_counting_evaluations,
    const string& description,
    utils::Verbosity verbosity)
    : description(description)
    , use_for_reporting_minima(use_for_reporting_minima)
    , use_for_boosting(use_for_boosting)
    , use_for_counting_evaluations(use_for_counting_evaluations)
    , log(utils::get_log_for_verbosity(verbosity))
{
}



const std::string& EvaluatorBase::get_description() const { return description; }

bool EvaluatorBase::is_used_for_reporting_minima() const
{
    return use_for_reporting_minima;
}

bool EvaluatorBase::is_used_for_boosting() const { return use_for_boosting; }

bool EvaluatorBase::is_used_for_counting_evaluations() const
{
    return use_for_counting_evaluations;
}

void EvaluatorBase::report_value_for_initial_state(
    const EvaluationResult& result) const
{
    if (log.is_at_least_normal()) {
        assert(use_for_reporting_minima);
        log << "Initial heuristic value for " << description << ": ";
        if (result.is_infinite())
            log << "infinity";
        else
            log << result.get_evaluator_value();
        log << endl;
    }
}

void EvaluatorBase::report_new_minimum_value(const EvaluationResult& result) const
{
    if (log.is_at_least_normal()) {
        assert(use_for_reporting_minima);
        log << "New best heuristic value for " << description << ": "
            << result.get_evaluator_value() << endl;
    }
}

void add_evaluator_options_to_feature(
    plugins::Feature& feature,
    const string& description)
{
    feature.add_option<string>(
        "description",
        "description used to identify EvaluatorBase in logs",
        "\"" + description + "\"");
    utils::add_log_options_to_feature(feature);
}

tuple<string, utils::Verbosity>
get_evaluator_arguments_from_options(const plugins::Options& opts)
{
    return tuple_cat(
        make_tuple(opts.get<string>("description")),
        utils::get_log_arguments_from_options(opts));
}
