#ifndef SEARCH_UTILS_H
#define SEARCH_UTILS_H

#include <memory>

class ClassicalPlanningTask;
class Evaluator;
class SearchAlgorithm;

namespace tests {

/**
 * @brief Creates an A* search engine without log output.
 *
 * @ingroup classical_planning_utils
 */
std::unique_ptr<SearchAlgorithm> create_astar_search_engine(
    std::shared_ptr<ClassicalPlanningTask> task,
    std::shared_ptr<Evaluator> evaluator);

}

#endif // SEARCH_UTILS_H
