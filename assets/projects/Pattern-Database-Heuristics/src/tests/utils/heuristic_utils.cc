#include "tests/utils/heuristic_utils.h"

#include "downward/heuristic.h"
#include "downward/abstract_task.h"
#include "downward/state.h"

namespace tests {

int get_initial_state_estimate(
    const ClassicalPlanningTask& task,
    Heuristic& heuristic)
{
    return heuristic.compute_heuristic(task.get_initial_state());
}

} // namespace tests