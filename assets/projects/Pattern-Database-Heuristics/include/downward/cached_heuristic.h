#ifndef CACHED_EVALUATOR_H
#define CACHED_EVALUATOR_H

#include "downward/evaluator.h"
#include "downward/per_state_information.h"

class Heuristic;

class CachedHeuristic : public Evaluator {
    static constexpr int DEAD_END = -1;
    static constexpr int NO_VALUE = -2;

protected:
    struct HEntry {
        /* dirty is conceptually a bool, but Visual C++ does not support
           packing ints and bools together in a bitfield. */
        int h : 31;
        unsigned int dirty : 1;

        HEntry(int h, bool dirty)
            : h(h)
            , dirty(dirty)
        {
        }
    };
    static_assert(sizeof(HEntry) == 4, "HEntry has unexpected size.");

    const std::shared_ptr<Heuristic> child;

    /*
      Cache for saving h values
      Before accessing this cache always make sure that the
      cache_evaluator_values flag is set to true - as soon as the cache is
      accessed it will create entries for all existing states
    */
    PerStateInformation<HEntry> heuristic_cache;

public:
    explicit CachedHeuristic(std::shared_ptr<Heuristic> child);

    bool dead_ends_are_reliable() const override;

    void get_path_dependent_evaluators(std::set<Evaluator*>& evals) override;

    void notify_initial_state(const State& initial_state) override;

    void notify_state_transition(
        const State& parent_state,
        OperatorID op_id,
        const State& state) override;

    EvaluationResult compute_result(EvaluationContext& eval_context) override;

    std::vector<EvaluationResult>
    compute_results(std::vector<EvaluationContext>& eval_contexts) override;

    const std::string& get_description() const override;

    bool is_used_for_reporting_minima() const override;

    bool is_used_for_boosting() const override;

    bool is_used_for_counting_evaluations() const override;

    void report_value_for_initial_state(
        const EvaluationResult& result) const override;

    void
    report_new_minimum_value(const EvaluationResult& result) const override;

    bool is_estimate_cached(const State& state) const;

    int get_cached_estimate(const State& state) const;
};

#endif
