#include "downward/heuristics/ff_heuristic.h"

#include "downward/heuristics/best_supporter_function.h"

#include "downward/plugins/plugin.h"
#include "downward/state.h"
#include "downward/task_proxy.h"

#include <unordered_set>
#include <vector>

using namespace std;

namespace ff_heuristic {

class FFHeuristic : public DeleteRelaxedPlanHeuristic {
private:
    std::shared_ptr<BestSupporterFunction> best_supporter_function;

public:
    FFHeuristic(
        const std::shared_ptr<ClassicalPlanningTask>& task,
        std::shared_ptr<BestSupporterFunction> best_supporter_function);

    /**
     * @brief Returns a delete-relaxed plan paired with its cost, if existent,
     * and std::nullopt otherwise.
     *
     * @note Implementations are allowed to be non-deterministic, i.e., to
     * return different delete-relaxed plans when called multiple times on the
     * same state.
     *
     * @see DeleteRelaxedPlan
     */
    std::optional<DeleteRelaxedPlan>
    compute_relaxed_plan(const State& state) override;
};

// construction and destruction
FFHeuristic::FFHeuristic(
    const shared_ptr<ClassicalPlanningTask>& task,
    std::shared_ptr<BestSupporterFunction> best_supporter_function)
    : DeleteRelaxedPlanHeuristic(task),
      best_supporter_function(std::move(best_supporter_function))
{
}

std::optional<DeleteRelaxedPlan>
FFHeuristic::compute_relaxed_plan(const State& state)
{
    // Compute best supporters for the state
    best_supporter_function->compute_best_supporters(state);

    auto hash_fact = [](const FactPair& f) {
        return (f.var << 16) | (f.value & 0xFFFF);
    };

    // Track which facts are achieved and which operators are in the plan
    unordered_set<int> achieved_facts;
    unordered_set<int> operators_in_plan;
    
    // Initialize with facts true in initial state
    for (size_t var = 0; var < state.size(); ++var) {
        FactPair fact(var, state[var]);
        achieved_facts.insert(hash_fact(fact));
    }

    // Collect all operators needed via backward search
    vector<FactPair> facts_to_achieve;
    for (FactProxy goal : task->get_goal()) {
        FactPair goal_fact = goal.get_pair();
        if (!achieved_facts.count(hash_fact(goal_fact))) {
            facts_to_achieve.push_back(goal_fact);
        }
    }

    // Backward search to find all necessary operators
    unordered_set<int> visited_facts;
    while (!facts_to_achieve.empty()) {
        FactPair fact = facts_to_achieve.back();
        facts_to_achieve.pop_back();

        int fact_id = hash_fact(fact);
        if (visited_facts.count(fact_id)) {
            continue;
        }
        visited_facts.insert(fact_id);

        if (achieved_facts.count(fact_id)) {
            continue; // Already achieved
        }

        // Get best supporter
        auto best_supporter = best_supporter_function->get_best_supporter(fact);
        if (!best_supporter.has_value()) {
            return std::nullopt; // Dead end
        }

        OperatorID op_id = *best_supporter;
        int op_id_int = op_id.get_index();
        
        if (!operators_in_plan.count(op_id_int)) {
            operators_in_plan.insert(op_id_int);
            
            // Add preconditions to achieve
            OperatorProxy op = task->get_operators()[op_id];
            for (FactProxy precond : op.get_preconditions()) {
                facts_to_achieve.push_back(precond.get_pair());
            }
        }
    }

    // Now build properly ordered plan using forward search
    DeleteRelaxedPlan plan;
    plan.cost = 0;
    
    // Keep applying operators until goals are achieved
    while (true) {
        // Check if all goals achieved
        bool all_goals_achieved = true;
        for (FactProxy goal : task->get_goal()) {
            if (!achieved_facts.count(hash_fact(goal.get_pair()))) {
                all_goals_achieved = false;
                break;
            }
        }
        if (all_goals_achieved) {
            break;
        }

        // Find an applicable operator from our plan
        bool found_applicable = false;
        for (int op_id_int : operators_in_plan) {
            OperatorID op_id(op_id_int);
            OperatorProxy op = task->get_operators()[op_id];
            
            // Check if preconditions satisfied
            bool applicable = true;
            for (FactProxy precond : op.get_preconditions()) {
                if (!achieved_facts.count(hash_fact(precond.get_pair()))) {
                    applicable = false;
                    break;
                }
            }
            
            if (applicable) {
                // Apply operator
                plan.operators.push_back(op_id);
                plan.cost += op.get_cost();
                operators_in_plan.erase(op_id_int);
                
                // Add effects
                for (FactProxy effect : op.get_effects()) {
                    achieved_facts.insert(hash_fact(effect.get_pair()));
                }
                
                found_applicable = true;
                break;
            }
        }
        
        if (!found_applicable) {
            // No applicable operator found - should not happen if BSF is correct
            return std::nullopt;
        }
    }

    return plan;
}

std::unique_ptr<DeleteRelaxedPlanHeuristic> create_hff_heuristic(
    std::shared_ptr<ClassicalPlanningTask> task,
    std::shared_ptr<BestSupporterFunction> best_supporter_function)
{
    return std::make_unique<FFHeuristic>(
        std::move(task),
        std::move(best_supporter_function));
}

class FFHeuristicFeature
    : public plugins::TypedFeature<Evaluator, DeleteRelaxedPlanHeuristic> {
public:
    FFHeuristicFeature()
        : TypedFeature("ff")
    {
        document_title("FF heuristic");

        add_heuristic_options_to_feature(*this, "ff");
        this->add_option<std::shared_ptr<BestSupporterFunction>>(
            "bsf",
            "The best supporter function",
            "hmax_bsf()");

        document_language_support("action costs", "supported");
        document_language_support("conditional effects", "supported");
        document_language_support(
            "axioms",
            "supported (in the sense that the planner won't complain -- "
            "handling of axioms might be very stupid "
            "and even render the heuristic unsafe)");

        document_property("admissible", "no");
        document_property("consistent", "no");
        document_property("safe", "yes for domains without axioms");
        document_property("preferred operators", "yes");
    }

    virtual shared_ptr<DeleteRelaxedPlanHeuristic>
    create_component(const plugins::Options& opts, const utils::Context&)
        const override
    {
        return create_hff_heuristic(
            std::get<0>(get_heuristic_arguments_from_options(opts)),
            opts.get<std::shared_ptr<BestSupporterFunction>>("bsf"));
    }
};

static plugins::FeaturePlugin<FFHeuristicFeature> _plugin;
} // namespace ff_heuristic
