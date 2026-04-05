#include "tests/utils/search_utils.h"

#include "downward/search_algorithms/eager_search.h"
#include "downward/search_algorithms/search_common.h"

#include "downward/open_list_factory.h"

#include <set>

namespace tests {

std::unique_ptr<SearchAlgorithm> create_astar_search_engine(
    std::shared_ptr<ClassicalPlanningTask> task,
    std::shared_ptr<Evaluator> evaluator)
{
    auto [open_list_factory, eval] =
        search_common::create_astar_open_list_factory_and_f_eval(
            evaluator,
            utils::Verbosity::SILENT);
    return std::make_unique<eager_search::EagerSearch>(
        open_list_factory,
        true,
        eval,
        std::vector<std::shared_ptr<Evaluator>>{},
        std::shared_ptr<CachedHeuristic>(),
        std::move(task),
        OperatorCost::NORMAL,
        std::numeric_limits<int>::max(),
        std::numeric_limits<double>::infinity(),
        "astar",
        utils::Verbosity::SILENT);
}

}