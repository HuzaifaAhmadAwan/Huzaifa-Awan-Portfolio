#include "tests/domains/gripper.h"

#include "downward/utils/system.h"

#include <set>

namespace tests {

Gripper::Gripper(int num_rooms, int num_balls)
    : ClassicalPlanningDomain(3 + num_balls, (4 * num_balls + num_rooms) * num_rooms)
    , num_rooms(num_rooms)
    , num_balls(num_balls)
{
    // Variables
    {
        std::vector<std::string> fact_names;
        for (int i = 0; i != num_rooms; ++i) {
            fact_names.emplace_back(
                std::string("robot-at Room_") + std::to_string(i));
        }
        variable_infos[get_variable_robot_at()] =
            VariableInfo("robot-at", num_rooms, std::move(fact_names));
    }
    {
        std::vector<std::string> fact_names;
        for (int i = 0; i != num_balls; ++i) {
            fact_names.emplace_back(
                std::string("carry-left Ball_") + std::to_string(i));
        }
        fact_names.emplace_back("Nothing");

        variable_infos[get_variable_carry_left()] =
            VariableInfo("carry-left", num_balls + 1, std::move(fact_names));
    }
    {
        std::vector<std::string> fact_names;
        for (int i = 0; i != num_balls; ++i) {
            fact_names.emplace_back(
                std::string("carry-right Ball_") + std::to_string(i));
        }
        fact_names.emplace_back("Nothing");
        variable_infos[get_variable_carry_right()] =
            VariableInfo("carry-right", num_balls + 1, std::move(fact_names));
    }

    for (int b = 0; b != num_balls; ++b) {
        std::vector<std::string> fact_names;
        for (int i = 0; i != num_rooms; ++i) {
            fact_names.emplace_back(
                std::string("ball-") + std::to_string(b) +
                std::string("-at Room_") + std::to_string(i));
        }
        fact_names.emplace_back(
            std::string("ball-") + std::to_string(b) +
            std::string("-at Gripper"));

        variable_infos[get_variable_ball_at(b)] = VariableInfo(
            "ball-" + std::to_string(b) + "-at",
            num_rooms + 1,
            fact_names);
    }

    // Operators
    for (int r1 = 0; r1 != num_rooms; ++r1) {
        for (int r2 = 0; r2 != num_rooms; ++r2) {
            operators[get_operator_move_index(r1, r2)] = OperatorInfo(
                "move " + std::to_string(r1) + " " + std::to_string(r2),
                1,
                {get_fact_robot_at_room(r1)},
                {get_fact_robot_at_room(r2)});
        }
    }

    for (int b = 0; b != num_balls; ++b) {
        for (int r = 0; r != num_rooms; ++r) {
            operators[get_operator_pick_left_index(b, r)] = OperatorInfo(
                "pick-left Ball_" + std::to_string(b) + " Room_" +
                    std::to_string(r),
                1,
                {get_fact_ball_at_room(b, r),
                 get_fact_robot_at_room(r),
                 get_fact_carry_left_none()},
                {get_fact_ball_in_gripper(b), get_fact_carry_left_ball(b)});
            operators[get_operator_pick_right_index(b, r)] = OperatorInfo(
                "pick-right Ball_" + std::to_string(b) + " Room_" +
                    std::to_string(r),
                1,
                {get_fact_ball_at_room(b, r),
                 get_fact_robot_at_room(r),
                 get_fact_carry_right_none()},
                {get_fact_ball_in_gripper(b), get_fact_carry_right_ball(b)});
            operators[get_operator_drop_left_index(b, r)] = OperatorInfo(
                "drop-left Ball_" + std::to_string(b) + " Room_" +
                    std::to_string(r),
                1,
                {get_fact_carry_left_ball(b), get_fact_robot_at_room(r)},
                {get_fact_ball_at_room(b, r), get_fact_carry_left_none()});
            operators[get_operator_drop_right_index(b, r)] = OperatorInfo(
                "drop-right Ball_" + std::to_string(b) + " Room_" +
                    std::to_string(r),
                1,
                {get_fact_carry_right_ball(b), get_fact_robot_at_room(r)},
                {get_fact_ball_at_room(b, r), get_fact_carry_right_none()});
        }
    }
}

int Gripper::get_variable_robot_at() const
{
    return 0;
}

int Gripper::get_variable_carry_left() const
{
    return 1;
}

int Gripper::get_variable_carry_right() const
{
    return 2;
}

int Gripper::get_variable_ball_at(int ball_idx) const
{
    verify_ball_index(ball_idx);
    return 3 + ball_idx;
}

FactPair Gripper::get_fact_robot_at_room(int room_idx) const
{
    verify_room_index(room_idx);
    return {get_variable_robot_at(), room_idx};
}

FactPair Gripper::get_fact_carry_left_ball(int ball_idx) const
{
    verify_ball_index(ball_idx);
    return {get_variable_carry_left(), ball_idx};
}

FactPair Gripper::get_fact_carry_left_none() const
{
    return {get_variable_carry_left(), num_balls};
}

FactPair Gripper::get_fact_carry_right_ball(int ball_idx) const
{
    verify_ball_index(ball_idx);
    return {get_variable_carry_right(), ball_idx};
}

FactPair Gripper::get_fact_carry_right_none() const
{
    return {get_variable_carry_right(), num_balls};
}

FactPair Gripper::get_fact_ball_at_room(int ball_idx, int room_idx) const
{
    verify_room_index(room_idx);
    return {get_variable_ball_at(ball_idx), room_idx};
}

FactPair Gripper::get_fact_ball_in_gripper(int ball_idx) const
{
    return {get_variable_ball_at(ball_idx), num_rooms};
}

OperatorID Gripper::get_operator_move_id(int r1, int r2) const
{
    return OperatorID(get_operator_move_index(r1, r2));
}

OperatorID Gripper::get_operator_pick_left_id(int b, int r) const
{
    return OperatorID(get_operator_pick_left_index(b, r));
}

OperatorID Gripper::get_operator_pick_right_id(int b, int r) const
{
    return OperatorID(get_operator_pick_right_index(b, r));
}

OperatorID Gripper::get_operator_drop_left_id(int b, int r) const
{
    return OperatorID(get_operator_drop_left_index(b, r));
}

OperatorID Gripper::get_operator_drop_right_id(int b, int r) const
{
    return OperatorID(get_operator_drop_right_index(b, r));
}

int Gripper::get_operator_move_index(int r1, int r2) const
{
    return r1 + num_rooms * r2;
}

int Gripper::get_operator_pick_left_index(int b, int r) const
{
    return num_rooms * num_rooms + b + num_balls * r;
}

int Gripper::get_operator_pick_right_index(int b, int r) const
{
    return num_rooms * num_rooms + num_balls * num_rooms + b + num_balls * r;
}

int Gripper::get_operator_drop_left_index(int b, int r) const
{
    return num_rooms * num_rooms + 2 * num_balls * num_rooms + b +
           num_balls * r;
}

int Gripper::get_operator_drop_right_index(int b, int r) const
{
    return num_rooms * num_rooms + 3 * num_balls * num_rooms + b +
           num_balls * r;
}

void Gripper::verify_room_index(int room_idx) const
{
    if (room_idx < 0 || room_idx >= num_rooms) {
        ABORT("Room index out of range!");
    }
}

void Gripper::verify_ball_index(int ball_idx) const
{
    if (ball_idx < 0 || ball_idx >= num_balls) {
        ABORT("Ball index out of range!");
    }
}

} // namespace tests
