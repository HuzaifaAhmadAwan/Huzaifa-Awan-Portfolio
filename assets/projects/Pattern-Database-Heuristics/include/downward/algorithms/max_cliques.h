#ifndef ALGORITHMS_MAX_CLIQUES_H
#define ALGORITHMS_MAX_CLIQUES_H

#include <set>
#include <vector>

namespace max_cliques {

/**
 * @brief Computes the set of maximal cliques in an undirected graph.
 *
 * The input graph is specified as an adjacency list.
 * The list contains one set of connected nodes for each graph node,
 * numbered from 0 to \f$N - 1\f$, where \f$N\f$ is the number of nodes.
 * E.g. the input \f$[ \{1, 2\}, \{0, 2\}, \{0, 1\}, \emptyset ]\f$
 * corresponds to a graph with \f$N=4\f$ nodes, numbered 0, 1, 2 and 3.
 * The node 0 is connected to both 1 and 2, the node 1 is connected to 0 and 2,
 * node 2 is connected to 0 and 1, and node 3 is isolated (no edges).
 *
 * ```
 * 0 -- 1
 * |  /
 * | /
 * 2    3
 *```
 *
 * Returns the set of maximal cliques, where each maximal clique is a set of
 * nodes. In this case, \f$\{\{0, 1, 2\}, \{3\}\}\f$.
 * Note that the set of maximal cliques of the empty graph is
 * \f$\{ \emptyset \}\f$ and not \f$\emptyset\f$, in particular the set of
 * maximal cliques is never empty.
 *
 * @ingroup downward_other
 *
 * @warning The adjacency list must undirected, i.e., if node \f$v\f$ is
 * present in the set of adjacent nodes of \f$u\f$, then \f$u\f$ must also be
 * present in the set of adjacent nodes of \f$v\f$.
 */
extern std::set<std::set<int>>
compute_max_cliques(const std::vector<std::set<int>>& graph);

}

#endif
