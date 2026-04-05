#include "downward/heuristics/pdb_heuristic.h"

#include "downward/heuristic.h"

#include "downward/pdbs/hstar.h"
#include "downward/pdbs/state_enumerator.h"
#include "downward/tasks/syntactic_projection.h"

#include "downward/state.h"
#include "downward/plugins/plugin.h"

#include <memory>
#include <vector>

using namespace std;

namespace pdbs {

class PDBHeuristic : public Heuristic {
private:
    std::shared_ptr<tasks::SyntacticProjection> projection_;
    std::shared_ptr<StateEnumerator> abstract_enumerator_;
    std::vector<int> h_star_table_;

public:
    PDBHeuristic(
        const std::shared_ptr<ClassicalPlanningTask>& task,
        const Pattern& pattern);

    int compute_heuristic(const State& ancestor_state) override;
};

PDBHeuristic::PDBHeuristic(
    const shared_ptr<ClassicalPlanningTask>& task,
    const Pattern& pattern)
    : Heuristic(task)
{
    // Create the syntactic projection for the given pattern
    projection_ = std::make_shared<tasks::SyntacticProjection>(*task, pattern);
    
    // Create a state enumerator for the abstract task
    abstract_enumerator_ = std::make_shared<StateEnumerator>(projection_);
    
    // Pre-compute the h* table for all abstract states
    h_star_table_ = compute_hstar_table(*projection_, *abstract_enumerator_);
}

int PDBHeuristic::compute_heuristic(const State& state)
{
    // Compute the abstract state
    State abstract_state = projection_->compute_abstract_state(state);
    
    // Get the index of the abstract state
    std::size_t abstract_idx = abstract_enumerator_->get_index(abstract_state);
    
    // Look up and return the h* value
    return h_star_table_[abstract_idx];
}

std::unique_ptr<Heuristic> create_pdb_heuristic(
    std::shared_ptr<ClassicalPlanningTask> task,
    std::vector<int> pattern)
{
    std::ranges::sort(pattern);

    return std::make_unique<pdbs::PDBHeuristic>(
        std::move(task),
        std::move(pattern));
}

class PDBHeuristicFeature : public plugins::TypedFeature<Evaluator, Heuristic> {
public:
    PDBHeuristicFeature()
        : TypedFeature("pdb")
    {
        document_subcategory("heuristics_pdb");
        document_title("Pattern database heuristic");
        document_synopsis("TODO");

        add_list_option<int>("pattern", "pattern for the PDB heuristic", "[]");
        add_heuristic_options_to_feature(*this, "pdb");

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
        return create_pdb_heuristic(
            std::get<0>(get_heuristic_arguments_from_options(opts)),
            opts.get_list<int>("pattern"));
    }
};

static plugins::FeaturePlugin<PDBHeuristicFeature> _plugin;
} // namespace pdbs
