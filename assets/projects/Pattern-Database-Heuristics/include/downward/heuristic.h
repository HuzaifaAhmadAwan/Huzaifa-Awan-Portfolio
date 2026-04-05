#ifndef HEURISTIC_H
#define HEURISTIC_H

#include "downward/evaluator_base.h"
#include "downward/task_proxy.h"

#include <memory>
#include <vector>

class ClassicalPlanningTask;

namespace plugins {
class Feature;
class Options;
} // namespace plugins


/**
 * @brief Represents a classical planning heuristic for a classical
 * planning task.
 *
 * @ingroup heuristics
 */
class Heuristic : public EvaluatorBase {
public:
    /**
     * @brief Represents a heuristic value of \f$\infty\f$.
     *
     * This special value must be returned by \ref compute_heuristic when the
     * heuristic value for the state is \f$\infty\f$.
     *
     * @attention Please take note that integer arithmetic involving this
     * special value does not behave as if it was truly \f$\infty\f$!
     * This is merely an indicator value.
     */
    static constexpr int DEAD_END = -1;

protected:
    /// The classical planning task this heuristic depends on.
    const std::shared_ptr<ClassicalPlanningTask> task;

public:
    /**
     * @brief Constructs the heuristic for the given planning task.
     *
     * @param task The planning task this heuristic depends on.
     */
    explicit Heuristic(const std::shared_ptr<ClassicalPlanningTask>& task);

    virtual ~Heuristic() override;

    /// Computes the heuristic value \f$h(s)\f$ for a given state \f$s\f$ of
    /// the planning task. If the heuristic value is infinite, returns
    /// Heuristic::DEAD_END.
    virtual int compute_heuristic(const State& ancestor_state) = 0;

    virtual EvaluationResult
    compute_result(EvaluationContext& eval_context) override;
};

extern void add_heuristic_options_to_feature(
    plugins::Feature& feature,
    const std::string&);
extern std::
    tuple<std::shared_ptr<ClassicalPlanningTask>>
    get_heuristic_arguments_from_options(const plugins::Options& opts);

#endif
