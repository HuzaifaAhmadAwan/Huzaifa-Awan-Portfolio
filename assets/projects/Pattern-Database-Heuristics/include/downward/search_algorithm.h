#ifndef SEARCH_ALGORITHM_H
#define SEARCH_ALGORITHM_H

#include "downward/operator_cost.h"
#include "downward/operator_id.h"
#include "downward/plan_manager.h"
#include "downward/search_progress.h"
#include "downward/search_space.h"
#include "downward/search_statistics.h"
#include "downward/state_registry.h"
#include "downward/task_proxy.h"

#include "downward/utils/countdown_timer.h"
#include "downward/utils/logging.h"

#include <optional>
#include <vector>

namespace plugins {
class Options;
class Feature;
} // namespace plugins

namespace ordered_set {
template <typename T>
class OrderedSet;
}

namespace successor_generator {
class SuccessorGenerator;
}

enum SearchStatus { IN_PROGRESS, TIMEOUT, FAILED, SOLVED };

class SearchAlgorithm {
    std::string description;
    SearchStatus status;
    bool solution_found;
    Plan plan;

protected:
    // Hold a reference to the task implementation and pass it to objects that
    // need it.
    const std::shared_ptr<ClassicalPlanningTask> task;

    mutable utils::LogProxy log;
    PlanManager plan_manager;
    StateRegistry state_registry;
    const successor_generator::SuccessorGenerator& successor_generator;
    SearchSpace search_space;
    SearchProgress search_progress;
    SearchStatistics statistics;
    int bound;
    OperatorCost cost_type;
    bool is_unit_cost;
    double max_time;

    // NeuralFD hack
    StateID goal_id = StateID::no_state;
    std::optional<utils::CountdownTimer> timer = std::nullopt;

    virtual void initialize() {}
    virtual SearchStatus step() = 0;

    void set_plan(const Plan& plan);
    bool check_goal_and_set_plan(const State& state);
    int get_adjusted_cost(const OperatorProxy& op) const;

public:
    SearchAlgorithm(
        std::shared_ptr<ClassicalPlanningTask> task,
        OperatorCost cost_type,
        int bound,
        double max_time,
        const std::string& description,
        utils::Verbosity verbosity);

    // TODO options object is needed for iterated search, the prototype for
    //  issue559 resolves this
    explicit SearchAlgorithm(const plugins::Options& opts);

    virtual ~SearchAlgorithm();
    virtual void print_statistics() const = 0;
    virtual void save_plan_if_necessary();
    bool found_solution() const;
    SearchStatus get_status() const;
    const Plan& get_plan() const;
    void search();
    const SearchStatistics& get_statistics() const { return statistics; }
    void set_bound(int b) { bound = b; }
    int get_bound() { return bound; }
    PlanManager& get_plan_manager() { return plan_manager; }
    std::string get_description() { return description; }

    // NeuralFD hacks

    const StateRegistry& get_state_registry() { return state_registry; }

    const SearchSpace& get_search_space() { return search_space; }

    const ClassicalPlanningTask& get_task() { return *task; }

    State get_goal_state()
    {
        assert(solution_found && goal_id != StateID::no_state);
        return state_registry.lookup_state(goal_id);
    }

    State get_goal_state() const
    {
        assert(solution_found && goal_id != StateID::no_state);
        return state_registry.lookup_state(goal_id);
    }
};

/*
  Print evaluator values of all evaluators evaluated in the evaluation context.
*/
extern void
print_initial_evaluator_values(const EvaluationContext& eval_context);

extern void add_search_algorithm_options_to_feature(
    plugins::Feature& feature,
    const std::string& description);
extern std::tuple<
    std::shared_ptr<ClassicalPlanningTask>,
    OperatorCost,
    int,
    double,
    std::string,
    utils::Verbosity>
get_search_algorithm_arguments_from_options(const plugins::Options& opts);
extern void add_successors_order_options_to_feature(plugins::Feature& feature);
extern std::tuple<bool, bool, int>
get_successors_order_arguments_from_options(const plugins::Options& opts);

#endif
