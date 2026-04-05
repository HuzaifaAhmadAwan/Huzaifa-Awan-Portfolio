#include <gtest/gtest.h>

#include "downward/heuristics/goal_count_heuristic.h"

#include "downward/heuristic.h"
#include "downward/search_algorithm.h"

#include "tests/domains/blocksworld.h"
#include "tests/domains/gripper.h"
#include "tests/domains/nomystery.h"
#include "tests/domains/sokoban.h"
#include "tests/domains/visitall.h"

#include "tests/utils/heuristic_utils.h"
#include "tests/utils/search_utils.h"
#include "tests/utils/task_utils.h"

using namespace goal_count_heuristic;
using namespace tests;

TEST(GoalCountTestsPublic, test_bw_goal_aware)
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

    auto task = create_task_from_domain(domain, initial_state, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 0);
}

TEST(GoalCountTestsPublic, test_bw_single_state)
{
    // 4 blocks
    BlocksWorld domain(4);

    /**
     * 1 2
     * 0 3
     */
    std::vector<FactPair> initial_state = {
        domain.get_fact_is_hand_empty(true),
        domain.get_fact_location_on_table(0),
        domain.get_fact_location_on_block(1, 0),
        domain.get_fact_location_on_block(2, 3),
        domain.get_fact_location_on_table(3),
        domain.get_fact_is_clear(0, false),
        domain.get_fact_is_clear(1, true),
        domain.get_fact_is_clear(2, true),
        domain.get_fact_is_clear(3, false)};

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

    auto task = tests::create_task_from_domain(domain, initial_state, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 2);
}

TEST(GoalCountTestsPublic, test_bw_single_state_reeval)
{
    // 4 blocks
    BlocksWorld domain(4);

    /**
     * 1 2
     * 0 3
     */
    std::vector<FactPair> initial_state = {
        domain.get_fact_is_hand_empty(true),
        domain.get_fact_location_on_table(0),
        domain.get_fact_location_on_block(1, 0),
        domain.get_fact_location_on_block(2, 3),
        domain.get_fact_location_on_table(3),
        domain.get_fact_is_clear(0, false),
        domain.get_fact_is_clear(1, true),
        domain.get_fact_is_clear(2, true),
        domain.get_fact_is_clear(3, false)};

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

    auto task = tests::create_task_from_domain(domain, initial_state, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 2);
    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 2);
}

TEST(GoalCountTestsPublic, test_bw_astar)
{
    // 4 blocks
    BlocksWorld domain(6);

    /**
     * 2
     * 1 4
     * 0 3 5
     */
    std::vector<FactPair> initial_state(
        {domain.get_fact_is_hand_empty(true),
         domain.get_fact_location_on_table(0),
         domain.get_fact_location_on_block(1, 0),
         domain.get_fact_location_on_block(2, 1),
         domain.get_fact_location_on_table(3),
         domain.get_fact_location_on_block(4, 3),
         domain.get_fact_location_on_table(5),
         domain.get_fact_is_clear(0, false),
         domain.get_fact_is_clear(1, false),
         domain.get_fact_is_clear(2, true),
         domain.get_fact_is_clear(3, false),
         domain.get_fact_is_clear(4, true),
         domain.get_fact_is_clear(5, true)});

    /**
     * 0
     * 4
     * 3
     * 5
     * 2
     * 1
     */
    std::vector<FactPair> goal(
        {domain.get_fact_location_on_block(0, 4),
         domain.get_fact_location_on_table(1),
         domain.get_fact_location_on_block(2, 1),
         domain.get_fact_location_on_block(3, 5),
         domain.get_fact_location_on_block(4, 3),
         domain.get_fact_location_on_block(5, 2)});

    auto task = tests::create_task_from_domain(domain, initial_state, goal);
    std::shared_ptr goal_count_heuristic = create_goal_count_heuristic(task);
    auto engine = create_astar_search_engine(task, goal_count_heuristic);

    engine->search();

    ASSERT_TRUE(engine->found_solution());
    ASSERT_EQ(engine->get_statistics().get_expanded(), 2783);

    const std::vector<OperatorID> expected_plan = {
        domain.get_operator_id_pick_from_block(2, 1),
        domain.get_operator_id_put_on_table(2),
        domain.get_operator_id_pick_from_block(1, 0),
        domain.get_operator_id_put_on_table(1),
        domain.get_operator_id_pick_from_table(2),
        domain.get_operator_id_put_on_block(2, 1),
        domain.get_operator_id_pick_from_table(5),
        domain.get_operator_id_put_on_block(5, 2),
        domain.get_operator_id_pick_from_block(4, 3),
        domain.get_operator_id_put_on_table(4),
        domain.get_operator_id_pick_from_table(3),
        domain.get_operator_id_put_on_block(3, 5),
        domain.get_operator_id_pick_from_table(4),
        domain.get_operator_id_put_on_block(4, 3),
        domain.get_operator_id_pick_from_table(0),
        domain.get_operator_id_put_on_block(0, 4)};

    ASSERT_EQ(engine->get_plan(), expected_plan);
}

TEST(GoalCountTestsPublic, test_gripper_goal_aware)
{
    // 2 rooms, 2 balls
    Gripper domain(2, 2);

    std::vector<FactPair> initial(
        {domain.get_fact_robot_at_room(0),
         domain.get_fact_carry_left_none(),
         domain.get_fact_carry_right_none(),
         domain.get_fact_ball_at_room(0, 1),
         domain.get_fact_ball_at_room(1, 1)});

    std::vector<FactPair> goal(
        {domain.get_fact_ball_at_room(0, 1),
         domain.get_fact_ball_at_room(1, 1)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 0);
}

TEST(GoalCountTestsPublic, test_gripper_single_state)
{
    // 2 rooms, 2 balls
    Gripper domain(2, 2);

    std::vector<FactPair> initial(
        {domain.get_fact_robot_at_room(0),
         domain.get_fact_carry_left_none(),
         domain.get_fact_carry_right_none(),
         domain.get_fact_ball_at_room(0, 0),
         domain.get_fact_ball_at_room(1, 0)});

    std::vector<FactPair> goal(
        {domain.get_fact_ball_at_room(0, 1),
         domain.get_fact_ball_at_room(1, 1)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 2);
}

TEST(GoalCountTestsPublic, test_gripper_single_state_reeval)
{
    // 2 rooms, 2 balls
    Gripper domain(2, 2);

    std::vector<FactPair> initial(
        {domain.get_fact_robot_at_room(0),
         domain.get_fact_carry_left_none(),
         domain.get_fact_carry_right_none(),
         domain.get_fact_ball_at_room(0, 0),
         domain.get_fact_ball_at_room(1, 0)});

    std::vector<FactPair> goal(
        {domain.get_fact_ball_at_room(0, 1),
         domain.get_fact_ball_at_room(1, 1)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 2);
    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 2);
}

TEST(GoalCountTestsPublic, test_gripper_astar)
{
    // 2 rooms, 2 balls
    Gripper domain(2, 2);

    std::vector<FactPair> initial(
        {domain.get_fact_robot_at_room(0),
         domain.get_fact_carry_left_none(),
         domain.get_fact_carry_right_none(),
         domain.get_fact_ball_at_room(0, 0),
         domain.get_fact_ball_at_room(1, 0)});

    std::vector<FactPair> goal(
        {domain.get_fact_ball_at_room(0, 1),
         domain.get_fact_ball_at_room(1, 1)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    std::shared_ptr goal_count_heuristic = create_goal_count_heuristic(task);
    auto engine = create_astar_search_engine(task, goal_count_heuristic);

    engine->search();

    ASSERT_TRUE(engine->found_solution());
    ASSERT_EQ(engine->get_statistics().get_expanded(), 19);

    const std::vector<OperatorID> expected_plan = {
        domain.get_operator_pick_left_id(0, 0),
        domain.get_operator_pick_right_id(1, 0),
        domain.get_operator_move_id(0, 1),
        domain.get_operator_drop_left_id(0, 1),
        domain.get_operator_drop_right_id(1, 1)};

    ASSERT_EQ(engine->get_plan(), expected_plan);
}

TEST(GoalCountTestsPublic, test_visitall_goal_aware)
{
    // 2 rooms, 2 balls
    VisitAll domain(2, 2);

    std::vector<FactPair> initial(
        {domain.get_fact_robot_at_square(0, 0),
         domain.get_fact_square_visited(0, 0, true),
         domain.get_fact_square_visited(0, 1, true),
         domain.get_fact_square_visited(1, 0, true),
         domain.get_fact_square_visited(1, 1, true)});

    std::vector<FactPair> goal(
        {domain.get_fact_square_visited(0, 0, true),
         domain.get_fact_square_visited(0, 1, true),
         domain.get_fact_square_visited(1, 0, true),
         domain.get_fact_square_visited(1, 1, true)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 0);
}

TEST(GoalCountTestsPublic, test_visitall_single_state)
{
    // 2 rooms, 2 balls
    VisitAll domain(2, 2);

    std::vector<FactPair> initial(
        {domain.get_fact_robot_at_square(0, 0),
         domain.get_fact_square_visited(0, 0, false),
         domain.get_fact_square_visited(0, 1, true),
         domain.get_fact_square_visited(1, 0, false),
         domain.get_fact_square_visited(1, 1, true)});

    std::vector<FactPair> goal(
        {domain.get_fact_square_visited(0, 0, true),
         domain.get_fact_square_visited(0, 1, true),
         domain.get_fact_square_visited(1, 0, true),
         domain.get_fact_square_visited(1, 1, true)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 2);
}

TEST(GoalCountTestsPublic, test_visitall_single_state_reeval)
{
    // 2 rooms, 2 balls
    VisitAll domain(2, 2);

    std::vector<FactPair> initial(
        {domain.get_fact_robot_at_square(0, 0),
         domain.get_fact_square_visited(0, 0, false),
         domain.get_fact_square_visited(0, 1, true),
         domain.get_fact_square_visited(1, 0, false),
         domain.get_fact_square_visited(1, 1, true)});

    std::vector<FactPair> goal(
        {domain.get_fact_square_visited(0, 0, true),
         domain.get_fact_square_visited(0, 1, true),
         domain.get_fact_square_visited(1, 0, true),
         domain.get_fact_square_visited(1, 1, true)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 2);
    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 2);
}

TEST(GoalCountTestsPublic, test_visitall_astar)
{
    // 2 rooms, 2 balls
    VisitAll domain(2, 2);

    std::vector<FactPair> initial(
        {domain.get_fact_robot_at_square(0, 0),
         domain.get_fact_square_visited(0, 0, true),
         domain.get_fact_square_visited(0, 1, false),
         domain.get_fact_square_visited(1, 0, false),
         domain.get_fact_square_visited(1, 1, false)});

    std::vector<FactPair> goal(
        {domain.get_fact_square_visited(0, 0, true),
         domain.get_fact_square_visited(0, 1, true),
         domain.get_fact_square_visited(1, 0, true),
         domain.get_fact_square_visited(1, 1, true)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    std::shared_ptr goal_count_heuristic = create_goal_count_heuristic(task);
    auto engine = create_astar_search_engine(task, goal_count_heuristic);

    engine->search();

    ASSERT_TRUE(engine->found_solution());
    ASSERT_EQ(engine->get_statistics().get_expanded(), 4);

    const std::vector<OperatorID> expected_plan = {
        domain.get_operator_move_up_id(0, 0),
        domain.get_operator_move_right_id(0, 1),
        domain.get_operator_move_down_id(1, 1)};

    ASSERT_EQ(engine->get_plan(), expected_plan);
}

TEST(GoalCountTestsPublic, test_sokoban_goal_aware)
{
    const SokobanGrid playarea = {
        {' ', ' ', ' '},
        {' ', 'G', ' '},
        {' ', 'G', ' '}};

    // 2 boxes
    Sokoban domain(playarea, 2);

    std::vector<FactPair> initial(
        {domain.get_fact_player_at(0, 0),
         domain.get_fact_box_at(0, 1, 1),
         domain.get_fact_box_at(1, 1, 2),
         domain.get_fact_box_at_goal(0, true),
         domain.get_fact_box_at_goal(1, true),
         domain.get_fact_square_clear(0, 0, false),
         domain.get_fact_square_clear(0, 1, true),
         domain.get_fact_square_clear(0, 2, true),
         domain.get_fact_square_clear(1, 0, true),
         domain.get_fact_square_clear(1, 1, false),
         domain.get_fact_square_clear(1, 2, false),
         domain.get_fact_square_clear(2, 0, true),
         domain.get_fact_square_clear(2, 1, true),
         domain.get_fact_square_clear(2, 2, true)});

    std::vector<FactPair> goal(
        {domain.get_fact_box_at_goal(0, true),
         domain.get_fact_box_at_goal(1, true)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 0);
}

TEST(GoalCountTestsPublic, test_sokoban_single_state)
{
    const SokobanGrid playarea = {
        {' ', ' ', ' '},
        {' ', 'G', ' '},
        {' ', 'G', ' '}};

    // 2 boxes
    Sokoban domain(playarea, 2);

    std::vector<FactPair> initial(
        {domain.get_fact_player_at(0, 0),
         domain.get_fact_box_at(0, 1, 1),
         domain.get_fact_box_at(1, 2, 2),
         domain.get_fact_box_at_goal(0, true),
         domain.get_fact_box_at_goal(1, false),
         domain.get_fact_square_clear(0, 0, false),
         domain.get_fact_square_clear(0, 1, true),
         domain.get_fact_square_clear(0, 2, true),
         domain.get_fact_square_clear(1, 0, true),
         domain.get_fact_square_clear(1, 1, false),
         domain.get_fact_square_clear(1, 2, true),
         domain.get_fact_square_clear(2, 0, true),
         domain.get_fact_square_clear(2, 1, true),
         domain.get_fact_square_clear(2, 2, false)});

    std::vector<FactPair> goal(
        {domain.get_fact_box_at_goal(0, true),
         domain.get_fact_box_at_goal(1, true)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 1);
}

TEST(GoalCountTestsPublic, test_sokoban_single_state_reeval)
{
    const SokobanGrid playarea = {
        {' ', ' ', ' '},
        {' ', 'G', ' '},
        {' ', 'G', ' '}};

    // 2 boxes
    Sokoban domain(playarea, 2);

    std::vector<FactPair> initial(
        {domain.get_fact_player_at(0, 0),
         domain.get_fact_box_at(0, 1, 1),
         domain.get_fact_box_at(1, 2, 2),
         domain.get_fact_box_at_goal(0, true),
         domain.get_fact_box_at_goal(1, false),
         domain.get_fact_square_clear(0, 0, false),
         domain.get_fact_square_clear(0, 1, true),
         domain.get_fact_square_clear(0, 2, true),
         domain.get_fact_square_clear(1, 0, true),
         domain.get_fact_square_clear(1, 1, false),
         domain.get_fact_square_clear(1, 2, true),
         domain.get_fact_square_clear(2, 0, true),
         domain.get_fact_square_clear(2, 1, true),
         domain.get_fact_square_clear(2, 2, false)});

    std::vector<FactPair> goal(
        {domain.get_fact_box_at_goal(0, true),
         domain.get_fact_box_at_goal(1, true)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 1);
    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 1);
}

TEST(GoalCountTestsPublic, test_sokoban_astar)
{
    const SokobanGrid playarea = {
        {' ', ' ', ' '},
        {' ', ' ', 'G'},
        {' ', ' ', 'G'}};

    // 2 boxes
    Sokoban domain(playarea, 2);

    std::vector<FactPair> initial(
        {domain.get_fact_player_at(0, 0),
         domain.get_fact_box_at(0, 1, 0),
         domain.get_fact_box_at(1, 1, 1),
         domain.get_fact_box_at_goal(0, false),
         domain.get_fact_box_at_goal(1, false),
         domain.get_fact_square_clear(0, 0, false),
         domain.get_fact_square_clear(0, 1, true),
         domain.get_fact_square_clear(0, 2, true),
         domain.get_fact_square_clear(1, 0, false),
         domain.get_fact_square_clear(1, 1, false),
         domain.get_fact_square_clear(1, 2, true),
         domain.get_fact_square_clear(2, 0, true),
         domain.get_fact_square_clear(2, 1, true),
         domain.get_fact_square_clear(2, 2, true)});

    std::vector<FactPair> goal(
        {domain.get_fact_box_at_goal(0, true),
         domain.get_fact_box_at_goal(1, true)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    std::shared_ptr goal_count_heuristic = create_goal_count_heuristic(task);
    auto engine = create_astar_search_engine(task, goal_count_heuristic);

    engine->search();

    ASSERT_TRUE(engine->found_solution());
    ASSERT_EQ(engine->get_statistics().get_expanded(), 11);

    const std::vector<OperatorID> expected_plan = {
        domain.get_operator_push_right_id(0, 0, 0),
        domain.get_operator_move_left_id(1, 0),
        domain.get_operator_move_up_id(0, 0),
        domain.get_operator_push_right_id(1, 0, 1)};

    ASSERT_EQ(engine->get_plan(), expected_plan);
}

TEST(GoalCountTestsPublic, test_nomystery_goal_aware)
{
    // 0 -- 1 -- 2 -- 3
    std::set<RoadMapEdge> roadmap =
        {{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}};

    // 4 locations, 2 packages, truck capacity 2
    NoMystery domain(4, 2, 2, roadmap);

    // Package P1 at L1 and P2 at L2, truck starts at L0.
    std::vector<FactPair> initial(
        {domain.get_fact_truck_at_location(0),
         domain.get_fact_packages_loaded(0),
         domain.get_fact_package_at_location(0, 1),
         domain.get_fact_package_at_location(1, 2)});

    // Package P1 at L1 and P2 at L2
    std::vector<FactPair> goal(
        {domain.get_fact_package_at_location(0, 1),
         domain.get_fact_package_at_location(1, 2)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 0);
}

TEST(GoalCountTestsPublic, test_nomystery_single_state)
{
    // 4 -- 5 -- 6
    // |  /
    // | /
    // 0 -- 1 -- 2   3 (disconnected)
    std::set<RoadMapEdge> roadmap = {
        {0, 1},
        {0, 4},
        {0, 5},
        {1, 0},
        {1, 2},
        {2, 1},
        {4, 0},
        {4, 5},
        {5, 0},
        {5, 4},
        {5, 6},
        {6, 5}};

    // 7 locations, 3 packages, truck capacity 2
    NoMystery domain(7, 3, 2, roadmap);

    // Package P1 at L1, P2 at L4, P3 at L5, truck starts at L0.
    std::vector<FactPair> initial(
        {domain.get_fact_truck_at_location(0),
         domain.get_fact_packages_loaded(0),
         domain.get_fact_package_at_location(0, 1),
         domain.get_fact_package_at_location(1, 4),
         domain.get_fact_package_at_location(2, 5)});

    // Package P1 at L2, P2 and P3 at L6
    std::vector<FactPair> goal(
        {domain.get_fact_package_at_location(0, 2),
         domain.get_fact_package_at_location(1, 6),
         domain.get_fact_package_at_location(2, 6)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 3);
}

TEST(GoalCountTestsPublic, test_nomystery_single_state_reeval)
{
    // 4 -- 5 -- 6
    // |  /
    // | /
    // 0 -- 1 -- 2   3 (disconnected)
    std::set<RoadMapEdge> roadmap = {
        {0, 1},
        {0, 4},
        {0, 5},
        {1, 0},
        {1, 2},
        {2, 1},
        {4, 0},
        {4, 5},
        {5, 0},
        {5, 4},
        {5, 6},
        {6, 5}};

    // 7 locations, 3 packages, truck capacity 2
    NoMystery domain(7, 3, 2, roadmap);

    // Package P1 at L1, P2 at L4, P3 at L5, truck starts at L0.
    std::vector<FactPair> initial(
        {domain.get_fact_truck_at_location(0),
         domain.get_fact_packages_loaded(0),
         domain.get_fact_package_at_location(0, 1),
         domain.get_fact_package_at_location(1, 4),
         domain.get_fact_package_at_location(2, 5)});

    // Package P1 at L2, P2 and P3 at L6
    std::vector<FactPair> goal(
        {domain.get_fact_package_at_location(0, 2),
         domain.get_fact_package_at_location(1, 6),
         domain.get_fact_package_at_location(2, 6)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    auto goal_count_heuristic = create_goal_count_heuristic(task);

    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 3);
    ASSERT_EQ(get_initial_state_estimate(*task, *goal_count_heuristic), 3);
}

TEST(GoalCountTestsPublic, test_nomystery_astar)
{
    // 4 -- 5 -- 6
    // |  /    /
    // | /    /
    // 0 -- 1 -- 2   3 (disconnected)
    std::set<RoadMapEdge> roadmap = {
        {0, 1},
        {0, 4},
        {0, 5},
        {1, 0},
        {1, 2},
        {1, 6},
        {2, 1},
        {4, 0},
        {4, 5},
        {5, 0},
        {5, 4},
        {5, 6},
        {6, 1},
        {6, 5}};

    // 7 locations, 3 packages, truck capacity 2
    NoMystery domain(7, 3, 2, roadmap);

    // Package P1 at L1, P2 at L4, P3 at L5, truck starts at L0.
    std::vector<FactPair> initial(
        {domain.get_fact_truck_at_location(0),
         domain.get_fact_packages_loaded(0),
         domain.get_fact_package_at_location(0, 1),
         domain.get_fact_package_at_location(1, 4),
         domain.get_fact_package_at_location(2, 5)});

    // Package P1 at L2, P2 and P3 at L6
    std::vector<FactPair> goal(
        {domain.get_fact_package_at_location(0, 2),
         domain.get_fact_package_at_location(1, 6),
         domain.get_fact_package_at_location(2, 6)});

    auto task = tests::create_task_from_domain(domain, initial, goal);
    std::shared_ptr goal_count_heuristic = create_goal_count_heuristic(task);
    auto engine = create_astar_search_engine(task, goal_count_heuristic);

    engine->search();

    ASSERT_TRUE(engine->found_solution());
    ASSERT_EQ(engine->get_statistics().get_expanded(), 256);

    const std::vector<OperatorID> expected_plan = {
        domain.get_operator_drive_id(0, 4),
        domain.get_operator_load_package_id(4, 1, 0),
        domain.get_operator_drive_id(4, 5),
        domain.get_operator_load_package_id(5, 2, 1),
        domain.get_operator_drive_id(5, 6),
        domain.get_operator_unload_package_id(6, 1, 2),
        domain.get_operator_unload_package_id(6, 2, 1),
        domain.get_operator_drive_id(6, 1),
        domain.get_operator_load_package_id(1, 0, 0),
        domain.get_operator_drive_id(1, 2),
        domain.get_operator_unload_package_id(2, 0, 1)};

    ASSERT_EQ(engine->get_plan(), expected_plan);
}