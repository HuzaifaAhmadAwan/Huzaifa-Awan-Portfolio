#include <gtest/gtest.h>

#include "downward/heuristic.h"
#include "downward/state.h"

#include "downward/pdbs/hstar.h"
#include "downward/pdbs/state_enumerator.h"

#include "tests/domains/blocksworld.h"
#include "tests/domains/gripper.h"
#include "tests/domains/nomystery.h"
#include "tests/domains/sokoban.h"
#include "tests/domains/visitall.h"

#include "tests/utils/task_utils.h"

using namespace pdbs;
using namespace tests;

TEST(HStarTableTestsPublic, test_blocksworld)
{
    // 1 block
    BlocksWorld domain(1);

    /**
     * 0
     */
    std::vector<FactPair> initial_state = {
        domain.get_fact_is_hand_empty(true),
        domain.get_fact_location_on_table(0),
        domain.get_fact_is_clear(0, true)};

    /**
     * 0
     */
    std::vector<FactPair> goal = {domain.get_fact_location_on_table(0)};

    auto task = create_task_from_domain(domain, initial_state, goal);

    StateEnumerator enumerator(task);

    std::vector<int> table = pdbs::compute_hstar_table(*task, enumerator);
    std::vector<int> expected_table = {
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        0,
        0,
        1,
        1,
        Heuristic::DEAD_END,
        2,
        0,
        0,
        1,
        1};

    ASSERT_EQ(table.size(), expected_table.size());

    for (std::size_t i = 0; i != enumerator.num_states(); ++i) {
        const auto value = table[i];
        const auto expected_value = expected_table[i];
        if (table[i] != expected_table[i]) {
            const State state = enumerator.get_state(i);
            const StateFactsProxy facts = task->get_state_facts(state);
            FAIL() << "Mismatch in computed h* value for state " << facts
                   << " (index " << i << "), expected " << expected_value
                   << " but got " << value;
        }
    }
}

TEST(HStarTableTestsPublic, test_gripper)
{
    // 1 room, 1 ball
    Gripper domain(1, 1);

    std::vector<FactPair> initial_state(
        {domain.get_fact_robot_at_room(0),
         domain.get_fact_carry_left_none(),
         domain.get_fact_carry_right_none(),
         domain.get_fact_ball_at_room(0, 0)});

    std::vector<FactPair> goal({domain.get_fact_ball_at_room(0, 0)});

    auto task = create_task_from_domain(domain, initial_state, goal);

    StateEnumerator enumerator(task);

    std::vector<int> table = pdbs::compute_hstar_table(*task, enumerator);
    std::vector<int> expected_table =
        {0, 0, 0, 0, 1, 1, 1, Heuristic::DEAD_END};

    ASSERT_EQ(table.size(), expected_table.size());

    for (std::size_t i = 0; i != enumerator.num_states(); ++i) {
        const auto value = table[i];
        const auto expected_value = expected_table[i];
        if (table[i] != expected_table[i]) {
            const State state = enumerator.get_state(i);
            const StateFactsProxy facts = task->get_state_facts(state);
            FAIL() << "Mismatch in computed h* value for state " << facts
                   << " (index " << i << "), expected " << expected_value
                   << " but got " << value;
        }
    }
}

TEST(HStarTableTestsPublic, test_visitall)
{
    // 1x2 grid
    VisitAll domain(1, 2);

    std::vector<FactPair> initial_state(
        {domain.get_fact_robot_at_square(0, 0),
         domain.get_fact_square_visited(0, 0, true),
         domain.get_fact_square_visited(0, 1, true)});

    std::vector<FactPair> goal(
        {domain.get_fact_square_visited(0, 0, true),
         domain.get_fact_square_visited(0, 1, true)});

    auto task = create_task_from_domain(domain, initial_state, goal);

    StateEnumerator enumerator(task);

    std::vector<int> table = pdbs::compute_hstar_table(*task, enumerator);
    std::vector<int> expected_table = {2, 2, 1, 2, 2, 1, 0, 0};

    ASSERT_EQ(table.size(), expected_table.size());

    for (std::size_t i = 0; i != enumerator.num_states(); ++i) {
        const auto value = table[i];
        const auto expected_value = expected_table[i];
        if (table[i] != expected_table[i]) {
            const State state = enumerator.get_state(i);
            const StateFactsProxy facts = task->get_state_facts(state);
            FAIL() << "Mismatch in computed h* value for state " << facts
                   << " (index " << i << "), expected " << expected_value
                   << " but got " << value;
        }
    }
}

TEST(HStarTableTestsPublic, test_sokoban)
{
    const SokobanGrid playarea = {{' ', 'G'}};

    // 1 box
    Sokoban domain(playarea, 1);

    std::vector<FactPair> initial_state(
        {domain.get_fact_player_at(0, 0),
         domain.get_fact_box_at(0, 1, 0),
         domain.get_fact_box_at_goal(0, true),
         domain.get_fact_square_clear(0, 0, false),
         domain.get_fact_square_clear(1, 0, false)});

    std::vector<FactPair> goal({domain.get_fact_box_at_goal(0, true)});

    auto task = create_task_from_domain(domain, initial_state, goal);

    StateEnumerator enumerator(task);

    std::vector<int> table = pdbs::compute_hstar_table(*task, enumerator);
    std::vector<int> expected_table = {
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        0,
        0,
        0,
        0,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        0,
        0,
        0,
        0,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        0,
        0,
        0,
        0,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        0,
        0,
        0,
        0};

    ASSERT_EQ(table.size(), expected_table.size());

    for (std::size_t i = 0; i != enumerator.num_states(); ++i) {
        const auto value = table[i];
        const auto expected_value = expected_table[i];
        if (table[i] != expected_table[i]) {
            const State state = enumerator.get_state(i);
            const StateFactsProxy facts = task->get_state_facts(state);
            FAIL() << "Mismatch in computed h* value for state " << facts
                   << " (index " << i << "), expected " << expected_value
                   << " but got " << value;
        }
    }
}

TEST(HStarTableTestsPublic, test_nomystery)
{
    // 0 -- 1 -- 2
    std::set<RoadMapEdge> roadmap = {{0, 1}, {1, 0}, {1, 2}, {2, 1}};

    // 3 locations, 1 package, truck capacity 1
    NoMystery domain(3, 1, 1, roadmap);

    // Package P1 at L1, truck starts at L0.
    std::vector<FactPair> initial_state(
        {domain.get_fact_truck_at_location(0),
         domain.get_fact_packages_loaded(0),
         domain.get_fact_package_at_location(0, 1)});

    // Package P1 at L1 and P2 at L2
    std::vector<FactPair> goal({domain.get_fact_package_at_location(0, 2)});

    auto task = create_task_from_domain(domain, initial_state, goal);

    StateEnumerator enumerator(task);

    std::vector<int> table = pdbs::compute_hstar_table(*task, enumerator);
    std::vector<int> expected_table = {
        4,
        5,
        6,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        4,
        3,
        4,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        0,
        0,
        0,
        0,
        0,
        0,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        Heuristic::DEAD_END,
        3,
        2,
        1};

    ASSERT_EQ(table.size(), expected_table.size());

    for (std::size_t i = 0; i != enumerator.num_states(); ++i) {
        const auto value = table[i];
        const auto expected_value = expected_table[i];
        if (table[i] != expected_table[i]) {
            const State state = enumerator.get_state(i);
            const StateFactsProxy facts = task->get_state_facts(state);
            FAIL() << "Mismatch in computed h* value for state " << facts
                   << " (index " << i << "), expected " << expected_value
                   << " but got " << value;
        }
    }
}