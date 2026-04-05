#ifndef EVALUATOR_BASE_H
#define EVALUATOR_BASE_H

#include "downward/evaluator.h"

#include "downward/utils/logging.h"

class EvaluationContext;
class State;

class EvaluatorBase : public Evaluator {
    const std::string description;
    const bool use_for_reporting_minima;
    const bool use_for_boosting;
    const bool use_for_counting_evaluations;

protected:
    mutable utils::LogProxy log;

public:
    static constexpr int NO_VALUE = -2;

    EvaluatorBase(
        bool use_for_reporting_minima,
        bool use_for_boosting,
        bool use_for_counting_evaluations,
        const std::string& description,
        utils::Verbosity verbosity);

    const std::string& get_description() const override;
    bool is_used_for_reporting_minima() const override;
    bool is_used_for_boosting() const override;
    bool is_used_for_counting_evaluations() const override;

    void report_value_for_initial_state(
        const EvaluationResult& result) const override;

    void
    report_new_minimum_value(const EvaluationResult& result) const override;
};

extern void add_evaluator_options_to_feature(
    plugins::Feature& feature,
    const std::string& description);

extern std::tuple<std::string, utils::Verbosity>
get_evaluator_arguments_from_options(const plugins::Options& opts);

#endif
