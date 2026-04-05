#ifndef PDBS_PDB_HEURISTIC_H
#define PDBS_PDB_HEURISTIC_H

#include "downward/pdbs/types.h"

#include <memory>

class ClassicalPlanningTask;
class Heuristic;

namespace pdbs {

/**
 * @brief Creates the pattern database (PDB) heuristic for a given task and a
 * pattern.
 *
 * The PDB heuristic with respect to pattern \f$P\f$ computes the projection
 * heuristic
 * \f[
 * h^P(s) = h^\star_{\lts(\task|_P)}(s|_P)
 * \f]
 * where \f$\task_P\f$ is the syntactic projection of \f$\task\f$ with respect
 * to the pattern \f$P\f$ and \f$\lts(\task|_P)\f$ is the transition system
 * induced by the syntactic projection.
 *
 * Pattern database heuristics are implemented in a specific way.
 * Upon construction, the full perfect heuristic \f$h^\star_{\lts(\task|_P)}\f$
 * for \f$\lts(\task|_P)\f$ is computed and all abstract goal distances are
 * stored in a lookup table.
 * When an estimate for the (global) state \f$s\f$ is requested during search,
 * the abstract state \f$s|_P\f$ is computed and the abstract state distance
 * \f$h^\star_{\lts(\task|_P)}(s|_P)\f$ is looked up returned.
 * Querying an estimate thus takes constant time in a suitable implementation,
 * while the initial construction  is the bottleneck.
 *
 * @see tasks::SyntacticProjection
 *
 * @ingroup heuristics
 */
std::unique_ptr<Heuristic> create_pdb_heuristic(
    std::shared_ptr<ClassicalPlanningTask> task,
    pdbs::Pattern pattern);

} // namespace pdbs

#endif
