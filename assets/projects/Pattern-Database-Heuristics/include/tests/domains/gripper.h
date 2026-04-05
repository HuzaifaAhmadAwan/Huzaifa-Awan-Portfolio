#ifndef TESTS_DOMAINS_GRIPPER_H
#define TESTS_DOMAINS_GRIPPER_H

#include "tests/domains/classical_planning_domain.h"

#include "downward/task_proxy.h"

#include <vector>

namespace tests {

/**
 * @brief Models the planning domain Gripper.
 *
 * A Gripper problem consists of M rooms, N balls and a robot. The robot has two
 * grippers with which he can pick up and drop balls. The robot's task is to
 * transport the balls from one room to another. There are three kinds of
 * actions:
 *
 * 1. The robot can *move* from one room to another (all rooms are connected).
 * 2. The robot can *pick* a ball with one of his grippers, unless it already
 * holds a ball.
 * 3. The robot can *drop* a ball in one of his grippers.
 *
 * This environment is modelled as an FDR task as follows:
 *
 * Variables:
 * + Robot-At - location of the robot, one of the M rooms.
 * + Carry-Left - the object carried by the left gripper of the robot. Either
 *   'Nothing' or one of the N balls.
 * + Carry-Right - the object carried by the right gripper of the robot. Either
 *   'Nothing' or one of the N balls.
 * + Ball-At(b) - location of a ball. Either 'Gripper' (does not distinguish
 *   between left or right gripper!) or one of the M rooms.
 *
 * Operators:
 * + Move(r, r') - The robot moves from room r to room r'.
 *   - Precondition: Robot-At=r
 *   - Effect: Robot-At=r'
 * + Pick-left(b, r) - Picks a ball b in room r with the left gripper.
 *   - Precondition: Robot-At=r, Carry-Left=Nothing, Ball-At(b)=r
 *   - Effect: Carry-Left=b, Ball-At(b)=Gripper
 * + Pick-right(b, r) - Picks a ball b in room r with the right gripper.
 *   - Precondition: Robot-At=r, Carry-Right=Nothing, Ball-At(b)=r
 *   - Effect: Carry-Right=b, Ball-At(b)=Gripper
 * + Drop-left(b, r) - Drops a ball b in room r with the left gripper.
 *   - Precondition: Robot-At=r, Carry-Left=b
 *   - Effect: Carry-Left=Nothing, Ball-At(b)=r
 * + Drop-right(b, r) - Drops a ball b in room r with the right gripper.
 *   - Precondition: Robot-At=r, Carry-Right=b
 *   - Effect: Carry-Right=Nothing, Ball-At(b)=r
 *
 * All operators have unit costs.
 *
 *  @ingroup test_domains
 */
class Gripper : public ClassicalPlanningDomain {
    int num_rooms;
    int num_balls;

public:
    // Construct a gripper problem instance.
    explicit Gripper(int num_rooms, int num_balls);

    // Get the integer representation of a variable.
    int get_variable_robot_at() const;
    int get_variable_carry_left() const;
    int get_variable_carry_right() const;
    int get_variable_ball_at(int ball_idx) const;

    // Get the FactPair representation of a fact.
    FactPair get_fact_robot_at_room(int room_idx) const;
    FactPair get_fact_carry_left_ball(int ball_idx) const;
    FactPair get_fact_carry_left_none() const;
    FactPair get_fact_carry_right_ball(int ball_idx) const;
    FactPair get_fact_carry_right_none() const;
    FactPair get_fact_ball_at_room(int ball_idx, int room_idx) const;
    FactPair get_fact_ball_in_gripper(int ball_idx) const;

    // Get the OperatorID representation of an operator.
    OperatorID get_operator_move_id(int r1, int r2) const;
    OperatorID get_operator_pick_left_id(int b, int r) const;
    OperatorID get_operator_pick_right_id(int b, int r) const;
    OperatorID get_operator_drop_left_id(int b, int r) const;
    OperatorID get_operator_drop_right_id(int b, int r) const;

private:
    int get_operator_move_index(int r1, int r2) const;
    int get_operator_pick_left_index(int b, int r) const;
    int get_operator_pick_right_index(int b, int r) const;
    int get_operator_drop_left_index(int b, int r) const;
    int get_operator_drop_right_index(int b, int r) const;

    void verify_room_index(int room_idx) const;
    void verify_ball_index(int ball_idx) const;
};

} // namespace tests

#endif