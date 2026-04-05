#include <gtest/gtest.h>

#include "downward/heuristics/blind_search_heuristic.h"

#include "downward/heuristic.h"

#include "tests/domains/blocksworld.h"

#include "tests/utils/heuristic_utils.h"
#include "tests/utils/task_utils.h"

using namespace blind_search_heuristic;
using namespace tests;

TEST(BlindHeuristicTestsPublic, test_goal_aware)
{
    // 4 blocks
    BlocksWorld domain(4);

    /**
     * 3
     * 2
     * 1
     * 0
     */
    std::vector<FactPair> initial_state = {
        domain.get_fact_is_hand_empty(true),
        domain.get_fact_location_on_table(0),
        domain.get_fact_location_on_block(1, 0),
        domain.get_fact_location_on_block(2, 1),
        domain.get_fact_location_on_block(3, 2),
        domain.get_fact_is_clear(0, false),
        domain.get_fact_is_clear(1, false),
        domain.get_fact_is_clear(2, false),
        domain.get_fact_is_clear(3, true)};

    /**
     * 3
     * 2
     * 1
     * 0
     */
    std::vector<FactPair> goal = {
        domain.get_fact_location_on_table(0),
        domain.get_fact_location_on_block(1, 0),
        domain.get_fact_location_on_block(2, 1),
        domain.get_fact_location_on_block(3, 2)};

    std::shared_ptr task =
        create_task_from_domain(domain, initial_state, goal);
    auto blind_heuristic = create_blind_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *blind_heuristic), 0);
}

TEST(BlindHeuristicTestsPublic, test_non_goal_minimum_cost)
{
    // 4 blocks, cost of picking up block is 100, putting down is 10
    BlocksWorld domain(4, 100, 10);

    /**
     * 2
     * 1
     * 0 3
     */
    std::vector<FactPair> initial_state = {
        domain.get_fact_is_hand_empty(true),
        domain.get_fact_location_on_table(0),
        domain.get_fact_location_on_block(1, 0),
        domain.get_fact_location_on_block(2, 1),
        domain.get_fact_location_on_table(3),
        domain.get_fact_is_clear(0, false),
        domain.get_fact_is_clear(1, false),
        domain.get_fact_is_clear(2, true),
        domain.get_fact_is_clear(3, true)};

    /**
     * 3
     * 2
     * 1
     * 0
     */
    std::vector<FactPair> goal = {
        domain.get_fact_location_on_table(0),
        domain.get_fact_location_on_block(1, 0),
        domain.get_fact_location_on_block(2, 1),
        domain.get_fact_location_on_block(3, 2)};

    std::shared_ptr task =
        create_task_from_domain(domain, initial_state, goal);
    auto blind_heuristic = create_blind_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *blind_heuristic), 10);
}