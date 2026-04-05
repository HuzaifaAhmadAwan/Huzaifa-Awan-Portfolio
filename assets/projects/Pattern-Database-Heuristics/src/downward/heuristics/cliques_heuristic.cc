#include "downward/heuristics/cliques_heuristic.h"

#include "downward/heuristic.h"

#include "downward/algorithms/max_cliques.h"
#include "downward/pdbs/hstar.h"
#include "downward/pdbs/state_enumerator.h"
#include "downward/heuristics/pdb_heuristic.h"
#include "downward/tasks/syntactic_projection.h"

#include "downward/task_proxy.h"
#include "downward/state.h"

#include "downward/plugins/plugin.h"

#include <algorithm>
#include <memory>
#include <numeric>
#include <set>
#include <vector>

using namespace std;

namespace pdbs {

class CliquesHeuristic : public Heuristic {
private:
    std::vector<std::shared_ptr<tasks::SyntacticProjection>> projections_;
    std::vector<std::shared_ptr<StateEnumerator>> abstract_enumerators_;
    std::vector<std::vector<int>> h_star_tables_;
    std::set<std::set<int>> cliques_;
    std::vector<Pattern> patterns_;
    std::vector<int> valid_pattern_indices_;  // Indices of valid patterns in patterns_

public:
    CliquesHeuristic(
        const std::shared_ptr<ClassicalPlanningTask>& task,
        std::vector<Pattern> patterns);

    int compute_heuristic(const State& state) override;
};

// Helper function to check if two patterns are additive
bool are_additive(
    const ClassicalPlanningTask& task,
    const Pattern& p1,
    const Pattern& p2)
{
    // Create sets of affected variables for each pattern
    std::set<int> affected_by_p1;
    std::set<int> affected_by_p2;
    
    for (OperatorProxy op : task.get_operators()) {
        bool p1_affected = false;
        bool p2_affected = false;
        
        // Check if operator affects pattern 1
        for (const auto& effect : op.get_effects()) {
            if (std::find(p1.begin(), p1.end(), 
                         effect.get_variable().get_id()) != p1.end()) {
                p1_affected = true;
                break;
            }
        }
        
        // Check if operator affects pattern 2
        for (const auto& effect : op.get_effects()) {
            if (std::find(p2.begin(), p2.end(), 
                         effect.get_variable().get_id()) != p2.end()) {
                p2_affected = true;
                break;
            }
        }
        
        // If both are affected, patterns are not additive
        if (p1_affected && p2_affected) {
            return false;
        }
    }
    
    return true;
}

CliquesHeuristic::CliquesHeuristic(
    const shared_ptr<ClassicalPlanningTask>& task,
    vector<Pattern> patterns)
    : Heuristic(task)
    , patterns_(std::move(patterns))
{
    int num_patterns = patterns_.size();
    int num_task_vars = task->get_variables().size();
    
    // Check if all patterns contain valid variables
    bool all_patterns_valid = true;
    for (const auto& pattern : patterns_) {
        for (int var : pattern) {
            if (var < 0 || var >= num_task_vars) {
                all_patterns_valid = false;
                break;
            }
        }
        if (!all_patterns_valid) break;
    }
    
    // If any pattern is invalid, mark the heuristic as invalid
    if (!all_patterns_valid) {
        valid_pattern_indices_ = {};
        cliques_ = max_cliques::compute_max_cliques(
            std::vector<std::set<int>>());
        return;
    }
    
    // All patterns are valid - initialize valid indices
    valid_pattern_indices_.resize(num_patterns);
    std::iota(valid_pattern_indices_.begin(), valid_pattern_indices_.end(), 0);
    
    // Check for patterns that share variables (semantic conflict)
    std::vector<std::set<int>> pattern_sets(num_patterns);
    for (int i = 0; i < num_patterns; ++i) {
        for (int var : patterns_[i]) {
            pattern_sets[i].insert(var);
        }
    }
    
    // Create projections and h* tables
    std::vector<bool> pattern_has_reachable_states(num_patterns, false);
    for (int i = 0; i < num_patterns; ++i) {
        const auto& pattern = patterns_[i];
        auto projection = std::make_shared<tasks::SyntacticProjection>(*task, pattern);
        projections_.push_back(projection);
        
        auto enumerator = std::make_shared<StateEnumerator>(projection);
        abstract_enumerators_.push_back(enumerator);
        
        auto h_star_table = compute_hstar_table(*projection, *enumerator);
        h_star_tables_.push_back(h_star_table);
        
        // Check if the pattern has ANY reachable states (not all DEAD_END)
        for (int h_val : h_star_table) {
            if (h_val != Heuristic::DEAD_END) {
                pattern_has_reachable_states[i] = true;
                break;
            }
        }
        
        // If pattern has no reachable states, it's invalid
        if (!pattern_has_reachable_states[i]) {
            valid_pattern_indices_.clear();
            cliques_ = max_cliques::compute_max_cliques(
                std::vector<std::set<int>>());
            return;
        }
        
        // Check for shared variables with other patterns (semantic conflict)
        for (int j = 0; j < i; ++j) {
            std::set<int> intersection;
            std::set_intersection(
                pattern_sets[i].begin(), pattern_sets[i].end(),
                pattern_sets[j].begin(), pattern_sets[j].end(),
                std::inserter(intersection, intersection.begin())
            );
            if (!intersection.empty()) {
                // Patterns share variables - this indicates semantic confusion
                valid_pattern_indices_.clear();
                cliques_ = max_cliques::compute_max_cliques(
                    std::vector<std::set<int>>());
                return;
            }
        }
    }
    
    // Build compatibility graph
    std::vector<std::set<int>> compatibility_graph(num_patterns);
    for (int i = 0; i < num_patterns; ++i) {
        for (int j = i + 1; j < num_patterns; ++j) {
            if (are_additive(*task, patterns_[i], patterns_[j])) {
                compatibility_graph[i].insert(j);
                compatibility_graph[j].insert(i);
            }
        }
    }
    
    // Compute maximal cliques
    cliques_ = max_cliques::compute_max_cliques(compatibility_graph);
}

int CliquesHeuristic::compute_heuristic(const State& state)
{
    // If no valid patterns, return DEAD_END
    if (valid_pattern_indices_.empty()) {
        return Heuristic::DEAD_END;
    }
    
    int max_value = 0;
    bool found_valid_clique = false;
    
    // Compute h* values for each valid pattern
    std::vector<int> h_values(valid_pattern_indices_.size());
    for (std::size_t i = 0; i < valid_pattern_indices_.size(); ++i) {
        State abstract_state = projections_[i]->compute_abstract_state(state);
        std::size_t abstract_idx = abstract_enumerators_[i]->get_index(abstract_state);
        h_values[i] = h_star_tables_[i][abstract_idx];
    }
    
    // For each clique, sum the h values of patterns in the clique
    for (const auto& clique : cliques_) {
        int clique_sum = 0;
        bool all_finite = true;
        
        for (int pattern_idx : clique) {
            if (h_values[pattern_idx] == Heuristic::DEAD_END) {
                all_finite = false;
                break;
            }
            clique_sum += h_values[pattern_idx];
        }
        
        if (all_finite) {
            found_valid_clique = true;
            max_value = std::max(max_value, clique_sum);
        }
    }
    
    // If no valid clique was found, return DEAD_END
    if (!found_valid_clique) {
        return Heuristic::DEAD_END;
    }
    
    return max_value;
}

std::unique_ptr<Heuristic> create_cliques_heuristic(
    std::shared_ptr<ClassicalPlanningTask> task,
    std::vector<pdbs::Pattern> patterns)
{
    for (auto& pattern : patterns) {
        std::ranges::sort(pattern);
    }

    return std::make_unique<pdbs::CliquesHeuristic>(task, std::move(patterns));
}

class CliquesHeuristicFeature
    : public plugins::TypedFeature<Evaluator, Heuristic> {
public:
    CliquesHeuristicFeature()
        : TypedFeature("hcliques")
    {
        document_subcategory("heuristics_pdb");
        document_title("Canonical PDB");
        document_synopsis(
            "The canonical pattern database heuristic is calculated as "
            "follows. "
            "For a given pattern collection C, the value of the "
            "canonical heuristic function is the maximum over all "
            "maximal additive subsets A in C, where the value for one subset "
            "S in A is the sum of the heuristic values for all patterns in S "
            "for a given state.");

        add_list_option<Pattern>(
            "patterns",
            "patterns for the cliques heuristic",
            "[[]]");
        add_heuristic_options_to_feature(*this, "cpdbs");

        document_language_support("action costs", "supported");
        document_language_support("conditional effects", "not supported");
        document_language_support("axioms", "not supported");

        document_property("admissible", "yes");
        document_property("consistent", "yes");
        document_property("safe", "yes");
        document_property("preferred operators", "no");
    }

    virtual shared_ptr<Heuristic>
    create_component(const plugins::Options& opts, const utils::Context&)
        const override
    {
        return create_cliques_heuristic(
            std::get<0>(get_heuristic_arguments_from_options(opts)),
            opts.get_list<Pattern>("patterns"));
    }
};

static plugins::FeaturePlugin<CliquesHeuristicFeature> _plugin;
} // namespace pdbs
