#ifndef PDBS_CANONICAL_PDBS_HEURISTIC_H
#define PDBS_CANONICAL_PDBS_HEURISTIC_H

#include "downward/pdbs/types.h"

#include <memory>

class ClassicalPlanningTask;
class Heuristic;

namespace pdbs {

/**
 * @brief Creates the cliques heuristic for a given planning task and a given
 * set of patterns.
 *
 * The cliques heuristic with respect to a set of patterns \f$C\f$ computes the
 * heuristic
 * \f[
 * h^C(s) = \max_{D \in \mas(C)} \sum_{P \in D} h^P(s)
 * \f]
 * where \f$\mas(C)\f$ is the set of maximal additive subsets of \f$C\f$ and
 * \f$h^P\f$ is the projection / PDB heuristic for \f$P\f$.
 * A set of patterns is additive, if for every task operator
 * \f$a \in \operatorsof{\task}\f$, there is at most one \f$P \in C\f$ such that
 * \f$a\f$ affects \f$P\f$, in the sense that there is a variable
 * \f$v \in P\f$ on which \f$\eff_a\f$ is defined.
 *
 * To compute the set \f$\mas(C)\f$, we construct the so-called compatibility
 * graph for \f$C\f$ first.
 * This graph has the set of vertices \f$V = C\f$
 * (i.e., one node per pattern of \f$C\f$) and the edge relation
 * \f$E = \{ (P, Q) \mid P \text{ and } Q \text{ are additive} \}\f$.
 * Then, \f$\mas(C)\f$ consists of the set of maximal cliques of the
 * compatibility graph.
 *
 * @see max_cliques::compute_max_cliques
 * @see PDBHeuristic
 *
 * @ingroup heuristics
 */
std::unique_ptr<Heuristic> create_cliques_heuristic(
    std::shared_ptr<ClassicalPlanningTask> task,
    std::vector<pdbs::Pattern> patterns);

} // namespace pdbs

#endif
