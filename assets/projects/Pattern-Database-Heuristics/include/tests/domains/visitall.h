#ifndef TESTS_DOMAINS_VISITALL_H
#define TESTS_DOMAINS_VISITALL_H

#include "tests/domains/classical_planning_domain.h"

#include "downward/task_proxy.h"

#include <vector>

namespace tests {

/**
 * @brief Models the planning domain VisitAll.
 *
 * A VisitAll problem consists of a 2D grid with fixed width W and fixed height
 * H and a robot inside this grid. The task of the robot is to visit all or some
 * squares of the grid as fast as possible. There is only one action the robot
 * can do: He can move between any two horizontally or vertically, but not
 * diagonally adjacent squares in the grid.
 *
 * This environment is modelled as an FDR task as follows:
 *
 * Variables:
 * + Robot-At - position of the robot, a 2D coordinate <x, y> inside the grid.
 * + Visited(x, y) - whether the square <x, y> has been visited yet. True or
 *   false
 *
 * Operators:
 * + Move-left(x, y) - The robot moves left by one square. Only exists if x > 0.
 *   - Precondition: Robot-At=<x, y>
 *   - Effect: Robot-At=<x-1, y>, Visited(x-1, y)=true
 * + Move-up(x, y) - The robot moves up by one square. Only exists if y > 0.
 *   - Precondition: Robot-At=<x, y>
 *   - Effect: Robot-At=<x, y-1>, Visited(x, y-1)=true
 * + Move-right(x, y) - The robot moves right by one square. Only exists if
 *   x < W - 1.
 *   - Precondition: Robot-At=<x, y>
 *   - Effect: Robot-At=<x+1, y>, Visited(x+1, y)=true
 * + Move-down(x, y) - The robot moves down by one square. Only exists if
 *   y < H - 1.
 *   - Precondition: Robot-At=<x, y>
 *   - Effect: Robot-At=<x, y+1>, Visited(x, y+1)=true
 *
 * All operators have unit cost.
 *
 * @ingroup test_domains
 */
class VisitAll : public ClassicalPlanningDomain {
    int grid_width;
    int grid_height;

public:
    explicit VisitAll(int grid_width, int grid_height);

    // Get the integer representation of a variable.
    int get_variable_robot_at() const;
    int get_variable_square_visited(int x, int y) const;

    // Get the FactPair representation of a fact.
    FactPair get_fact_robot_at_square(int x, int y) const;
    FactPair get_fact_square_visited(int x, int y, bool was_visited) const;

    // Get the OperatorID representation of an operator.
    OperatorID get_operator_move_left_id(int x, int y) const;
    OperatorID get_operator_move_up_id(int x, int y) const;
    OperatorID get_operator_move_right_id(int x, int y) const;
    OperatorID get_operator_move_down_id(int x, int y) const;

private:
    int get_operator_move_left_index(int x, int y) const;
    int get_operator_move_up_index(int x, int y) const;
    int get_operator_move_right_index(int x, int y) const;
    int get_operator_move_down_index(int x, int y) const;

    void verify_coordinate(int x, int y) const;
};

} // namespace tests

#endif