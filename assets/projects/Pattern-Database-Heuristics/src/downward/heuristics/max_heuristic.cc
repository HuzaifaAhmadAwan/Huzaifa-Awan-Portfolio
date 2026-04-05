#include "downward/heuristics/max_heuristic.h"

#include "downward/heuristics/best_supporter_function.h"

#include "downward/heuristic.h"
#include "downward/state.h"
#include "downward/task_proxy.h"

#include "downward/plugins/plugin.h"

#include <unordered_map>
#include <limits>

using namespace std;

namespace max_heuristic {

// Helper function to hash facts
static inline int fact_hash(const FactPair &fact) {
    return (fact.var << 16) | (fact.value & 0xFFFF);
}

class HMaxHeuristic : public Heuristic {
private:
    mutable const State* current_state = nullptr;

public:
    explicit HMaxHeuristic(const std::shared_ptr<ClassicalPlanningTask>& task);

    int compute_heuristic(const State& state) override;
};

class HMaxBestSupporterFunction : public BestSupporterFunction {
private:
    std::shared_ptr<ClassicalPlanningTask> task;
    mutable unordered_map<int, int> best_supporters;
    mutable const State* current_state = nullptr;

public:
    HMaxBestSupporterFunction(
        const std::shared_ptr<ClassicalPlanningTask>& task);

    void compute_best_supporters(const State& state) override;

    std::optional<OperatorID> get_best_supporter(const FactPair& fact) override;
};

// construction and destruction
HMaxHeuristic::HMaxHeuristic(const shared_ptr<ClassicalPlanningTask>& task)
    : Heuristic(task)
{
}

int HMaxHeuristic::compute_heuristic(const State& state)
{
    current_state = &state;

    const int INF = std::numeric_limits<int>::max() / 4; // avoid overflow

    // map fact_id -> cost (uninitialized facts are implicitly INF)
    unordered_map<int, int> fact_cost;

    // Set initial-state facts to 0
    for (int var = 0; var < static_cast<int>(state.size()); ++var) {
        int val = state[var];
        int id = (var << 16) | (val & 0xFFFF);
        fact_cost[id] = 0;
    }

    bool changed = true;
    while (changed) {
        changed = false;
        // iterate over operators
        OperatorsProxy operators = task->get_operators();
        for (OperatorProxy op : operators) {
            // compute max cost among preconditions
            int max_pre = 0;
            bool all_pre_finite = true;

            OperatorPreconditionProxy preconditions = op.get_preconditions();
            for (FactProxy pre : preconditions) {
                int pid = fact_hash(pre.get_pair());
                auto it = fact_cost.find(pid);
                int pcost = (it == fact_cost.end()) ? INF : it->second;
                if (pcost == INF) {
                    all_pre_finite = false;
                    break;
                }
                if (pcost > max_pre) max_pre = pcost;
            }

            if (!all_pre_finite) continue;

            int new_cost = op.get_cost() + max_pre;
            // update all effect facts
            OperatorEffectProxy effects = op.get_effects();
            for (FactProxy eff : effects) {
                int eid = fact_hash(eff.get_pair());
                auto it = fact_cost.find(eid);
                int current_cost = (it == fact_cost.end()) ? INF : it->second;
                if (new_cost < current_cost) {
                    fact_cost[eid] = new_cost;
                    changed = true;
                }
            }
        }
    }

    // Now evaluate goal costs
    int max_hmax = 0;
    GoalProxy goal = task->get_goal();
    for (FactProxy g : goal) {
        int gid = fact_hash(g.get_pair());
        auto it = fact_cost.find(gid);
        int gcost = (it == fact_cost.end()) ? INF : it->second;
        if (gcost == INF) {
            current_state = nullptr;
            return Heuristic::DEAD_END;
        }
        if (gcost > max_hmax) max_hmax = gcost;
    }

    current_state = nullptr;
    return max_hmax;
}

HMaxBestSupporterFunction::HMaxBestSupporterFunction(
    const shared_ptr<ClassicalPlanningTask>& task)
    : task(task)
{
}

void HMaxBestSupporterFunction::compute_best_supporters(const State& state)
{
    current_state = &state;

    const int INF = std::numeric_limits<int>::max() / 4;

    // fact_id -> cost (uninitialized facts are implicitly INF)
    unordered_map<int, int> fact_cost;
    best_supporters.clear();

    // initial state facts cost = 0
    for (int var = 0; var < static_cast<int>(state.size()); ++var) {
        int val = state[var];
        int id = (var << 16) | (val & 0xFFFF);
        fact_cost[id] = 0;
        best_supporters[id] = -1; // no operator needed
    }

    bool changed = true;
    while (changed) {
        changed = false;
        OperatorsProxy operators = task->get_operators();
        for (OperatorProxy op : operators) {
            int max_pre = 0;
            bool all_pre = true;
            OperatorPreconditionProxy preconditions = op.get_preconditions();
            for (FactProxy pre : preconditions) {
                int pid = fact_hash(pre.get_pair());
                auto it = fact_cost.find(pid);
                int pcost = (it == fact_cost.end()) ? INF : it->second;
                if (pcost == INF) {
                    all_pre = false;
                    break;
                }
                if (pcost > max_pre) max_pre = pcost;
            }
            if (!all_pre) continue;

            int new_cost = op.get_cost() + max_pre;

            OperatorEffectProxy effects = op.get_effects();
            for (FactProxy eff : effects) {
                int eid = fact_hash(eff.get_pair());
                auto it = fact_cost.find(eid);
                int current_cost = (it == fact_cost.end()) ? INF : it->second;
                if (new_cost < current_cost) {
                    fact_cost[eid] = new_cost;
                    best_supporters[eid] = op.get_id();
                    changed = true;
                }
            }
        }
    }

    // After fixpoint, best_supporters contains operator IDs or -1.
    current_state = nullptr;
}

std::optional<OperatorID>
HMaxBestSupporterFunction::get_best_supporter(const FactPair& fact)
{
    int fid = fact_hash(fact);
    auto it = best_supporters.find(fid);
    if (it == best_supporters.end()) return std::nullopt;
    if (it->second < 0) return std::nullopt;
    return OperatorID(it->second);
}

std::unique_ptr<Heuristic>
create_hmax_heuristic(std::shared_ptr<ClassicalPlanningTask> task)
{
    return std::make_unique<HMaxHeuristic>(std::move(task));
}

std::unique_ptr<BestSupporterFunction>
create_hmax_best_supporter_function(std::shared_ptr<ClassicalPlanningTask> task)
{
    return std::make_unique<HMaxBestSupporterFunction>(std::move(task));
}

class HMaxHeuristicFeature
    : public plugins::TypedFeature<Evaluator, Heuristic> {
public:
    HMaxHeuristicFeature()
        : TypedFeature("hmax")
    {
        document_title("Max heuristic");

        add_heuristic_options_to_feature(*this, "hmax");

        document_language_support("action costs", "supported");
        document_language_support("conditional effects", "supported");
        document_language_support(
            "axioms",
            "supported (in the sense that the planner won't complain -- "
            "handling of axioms might be very stupid "
            "and even render the heuristic unsafe)");

        document_property("admissible", "yes for domains without axioms");
        document_property("consistent", "yes for domains without axioms");
        document_property("safe", "yes for domains without axioms");
        document_property("preferred operators", "no");
    }

    virtual shared_ptr<Heuristic>
    create_component(const plugins::Options& opts, const utils::Context&)
        const override
    {
        return create_hmax_heuristic(
            std::get<0>(get_heuristic_arguments_from_options(opts)));
    }
};

class HMaxBsfFeature
    : public plugins::
          TypedFeature<BestSupporterFunction, BestSupporterFunction> {
public:
    HMaxBsfFeature()
        : TypedFeature("hmax_bsf")
    {
        document_title("Max heuristic best supporter function");

        add_heuristic_options_to_feature(*this, "hmax_bsf");
    }

    virtual shared_ptr<BestSupporterFunction>
    create_component(const plugins::Options& opts, const utils::Context&)
        const override
    {
        return create_hmax_best_supporter_function(
            std::get<0>(get_heuristic_arguments_from_options(opts)));
    }
};

static plugins::FeaturePlugin<HMaxHeuristicFeature> _plugin;
static plugins::FeaturePlugin<HMaxBsfFeature> _plugin2;

} // namespace max_heuristic
