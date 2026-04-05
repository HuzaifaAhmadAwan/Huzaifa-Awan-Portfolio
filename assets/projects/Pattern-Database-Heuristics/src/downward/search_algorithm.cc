#include "downward/search_algorithm.h"

#include "downward/evaluation_context.h"
#include "downward/evaluator.h"

#include "downward/algorithms/ordered_set.h"
#include "downward/plugins/plugin.h"
#include "downward/task_utils/successor_generator.h"
#include "downward/task_utils/task_properties.h"
#include "downward/tasks/root_task.h"
#include "downward/utils/countdown_timer.h"
#include "downward/utils/rng_options.h"
#include "downward/utils/system.h"
#include "downward/utils/timer.h"

#include <cassert>
#include <iostream>
#include <limits>

using namespace std;
using utils::ExitCode;

static successor_generator::SuccessorGenerator&
get_successor_generator(const ClassicalPlanningTask& task, utils::LogProxy& log)
{
    if (log.is_at_least_normal()) {
        log << "Building successor generator..." << flush;
    }
    int peak_memory_before = utils::get_peak_memory_in_kb();
    utils::Timer successor_generator_timer;
    successor_generator::SuccessorGenerator& successor_generator =
        successor_generator::g_successor_generators[task];
    successor_generator_timer.stop();
    if (log.is_at_least_normal()) {
        log << "done!" << endl;
    }
    int peak_memory_after = utils::get_peak_memory_in_kb();
    int memory_diff = peak_memory_after - peak_memory_before;
    if (log.is_at_least_normal()) {
        log << "peak memory difference for successor generator creation: "
            << memory_diff << " KB" << endl
            << "time for successor generation creation: "
            << successor_generator_timer << endl;
    }
    return successor_generator;
}

SearchAlgorithm::SearchAlgorithm(
    std::shared_ptr<ClassicalPlanningTask> task,
    OperatorCost cost_type,
    int bound,
    double max_time,
    const string& description,
    utils::Verbosity verbosity)
    : description(description)
    , status(IN_PROGRESS)
    , solution_found(false)
    , task(std::move(task))
    , log(utils::get_log_for_verbosity(verbosity))
    , state_registry(*this->task)
    , successor_generator(get_successor_generator(*this->task, log))
    , search_space(state_registry, log)
    , statistics(log)
    , bound(bound)
    , cost_type(cost_type)
    , is_unit_cost(task_properties::is_unit_cost(*this->task))
    , max_time(max_time)
{
    if (bound < 0) {
        cerr << "error: negative cost bound " << bound << endl;
        utils::exit_with(ExitCode::SEARCH_INPUT_ERROR);
    }
    if (log.is_at_least_normal()) {
        task_properties::print_variable_statistics(*this->task, log);
    }
}

SearchAlgorithm::SearchAlgorithm(const plugins::Options& opts)
    : description(opts.get_unparsed_config())
    , status(IN_PROGRESS)
    , solution_found(false)
    , task(tasks::g_root_task)
    , log(utils::get_log_for_verbosity(opts.get<utils::Verbosity>("verbosity")))
    , state_registry(*task)
    , successor_generator(get_successor_generator(*task, log))
    , search_space(state_registry, log)
    , statistics(log)
    , cost_type(opts.get<OperatorCost>("cost_type"))
    , is_unit_cost(task_properties::is_unit_cost(*task))
    , max_time(opts.get<double>("max_time"))
{
    if (opts.get<int>("bound") < 0) {
        cerr << "error: negative cost bound " << opts.get<int>("bound") << endl;
        utils::exit_with(ExitCode::SEARCH_INPUT_ERROR);
    }
    bound = opts.get<int>("bound");
    if (log.is_at_least_normal()) {
        task_properties::print_variable_statistics(*task, log);
    }
}

SearchAlgorithm::~SearchAlgorithm()
{
}

bool SearchAlgorithm::found_solution() const
{
    return solution_found;
}

SearchStatus SearchAlgorithm::get_status() const
{
    return status;
}

const Plan& SearchAlgorithm::get_plan() const
{
    assert(solution_found);
    return plan;
}

void SearchAlgorithm::set_plan(const Plan& p)
{
    solution_found = true;
    plan = p;
}

void SearchAlgorithm::search()
{
    initialize();
    timer = utils::CountdownTimer(max_time);
    while (status == IN_PROGRESS) {
        status = step();
        if (timer->is_expired()) {
            if (log.is_at_least_normal()) {
                log << "Time limit reached. Abort search." << endl;
            }
            status = TIMEOUT;
            break;
        }
    }
    // TODO: Revise when and which search times are logged.
    if (log.is_at_least_normal()) {
        log << "Actual search time: " << timer->get_elapsed_time() << endl;
    }
    timer = std::nullopt;
}

bool SearchAlgorithm::check_goal_and_set_plan(const State& state)
{
    if (task_properties::is_goal_state(*task, state)) {
        if (log.is_at_least_normal()) {
            log << "Solution found!" << endl;
        }
        goal_id = state.get_id();
        Plan plan;
        search_space.trace_path(state, plan);
        set_plan(plan);
        return true;
    }
    return false;
}

void SearchAlgorithm::save_plan_if_necessary()
{
    if (found_solution()) {
        plan_manager.save_plan(get_plan(), *task);
    }
}

int SearchAlgorithm::get_adjusted_cost(const OperatorProxy& op) const
{
    return get_adjusted_action_cost(op, cost_type, is_unit_cost);
}

void print_initial_evaluator_values(const EvaluationContext& eval_context)
{
    eval_context.get_cache().for_each_evaluator_result(
        [](const Evaluator* eval, const EvaluationResult& result) {
            if (eval->is_used_for_reporting_minima()) {
                eval->report_value_for_initial_state(result);
            }
        });
}

void add_search_algorithm_options_to_feature(
    plugins::Feature& feature,
    const string& description)
{
    feature.add_option<std::shared_ptr<ClassicalPlanningTask>>(
        "transform",
        "the task relative to which search is performed",
        "no_transform()");
    ::add_cost_type_options_to_feature(feature);
    feature.add_option<int>(
        "bound",
        "exclusive depth bound on g-values. Cutoffs are always performed "
        "according to "
        "the real cost, regardless of the cost_type parameter",
        "infinity");
    feature.add_option<double>(
        "max_time",
        "maximum time in seconds the search is allowed to run for. The "
        "timeout is only checked after each complete search step "
        "(usually a node expansion), so the actual runtime can be arbitrarily "
        "longer. Therefore, this parameter should not be used for "
        "time-limiting "
        "experiments. Timed-out searches are treated as failed searches, "
        "just like incomplete search algorithms that exhaust their search "
        "space.",
        "infinity");
    feature.add_option<string>(
        "description",
        "description used to identify search algorithm in logs",
        "\"" + description + "\"");
    utils::add_log_options_to_feature(feature);
}

tuple<
    std::shared_ptr<ClassicalPlanningTask>,
    OperatorCost,
    int,
    double,
    string,
    utils::Verbosity>
get_search_algorithm_arguments_from_options(const plugins::Options& opts)
{
    return tuple_cat(
        make_tuple(opts.get<std::shared_ptr<ClassicalPlanningTask>>("transform")),
        ::get_cost_type_arguments_from_options(opts),
        make_tuple(
            opts.get<int>("bound"),
            opts.get<double>("max_time"),
            opts.get<string>("description")),
        utils::get_log_arguments_from_options(opts));
}

/* Method doesn't belong here because it's only useful for certain derived
   classes.
   TODO: Figure out where it belongs and move it there. */
void add_successors_order_options_to_feature(plugins::Feature& feature)
{
    vector<string> options;
    feature.add_option<bool>(
        "randomize_successors",
        "randomize the order in which successors are generated",
        "false");
    feature.add_option<bool>(
        "preferred_successors_first",
        "consider preferred operators first",
        "false");
    feature.document_note(
        "Successor ordering",
        "When using randomize_successors=true and "
        "preferred_successors_first=true, randomization happens before "
        "preferred operators are moved to the front.");
    utils::add_rng_options_to_feature(feature);
}

tuple<bool, bool, int>
get_successors_order_arguments_from_options(const plugins::Options& opts)
{
    return tuple_cat(
        make_tuple(
            opts.get<bool>("randomize_successors"),
            opts.get<bool>("preferred_successors_first")),
        utils::get_rng_arguments_from_options(opts));
}

static class SearchAlgorithmCategoryPlugin
    : public plugins::TypedCategoryPlugin<SearchAlgorithm> {
public:
    SearchAlgorithmCategoryPlugin()
        : TypedCategoryPlugin("SearchAlgorithm")
    {
        // TODO: Replace add synopsis for the wiki page.
        // document_synopsis("...");
    }
} _category_plugin;
