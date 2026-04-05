#include "tests/domains/visitall.h"

#include <exception>
#include <set>

namespace tests {

VisitAll::VisitAll(int grid_width, int grid_height)
    : ClassicalPlanningDomain(
          1 + grid_width * grid_height,
          2 * (grid_width - 1) * grid_height +
              2 * grid_width * (grid_height - 1))
    , grid_width(grid_width)
    , grid_height(grid_height)
{
    // Variables
    variable_infos[get_variable_robot_at()].name = "robot-at";
    variable_infos[get_variable_robot_at()].domain_size =
        grid_width * grid_height;

    for (int x = 0; x != grid_width; ++x) {
        for (int y = 0; y != grid_height; ++y) {
            variable_infos[get_variable_robot_at()].fact_names.emplace_back(
                "robot-at <" + std::to_string(x) + ", " + std::to_string(y) +
                ">");

            variable_infos[get_variable_square_visited(x, y)].name =
                std::string("visited-") + std::to_string(x) + std::string("-") +
                std::to_string(y);

            variable_infos[get_variable_square_visited(x, y)].domain_size = 2;
            variable_infos[get_variable_square_visited(x, y)]
                .fact_names.emplace_back(
                    std::string("visited-") + std::to_string(x) +
                    std::string("-") + std::to_string(y) + " false");
            variable_infos[get_variable_square_visited(x, y)]
                .fact_names.emplace_back(
                    std::string("visited-") + std::to_string(x) +
                    std::string("-") + std::to_string(y) + " true");
        }
    }

    // Operators

    // Left
    for (int x = 1; x < grid_width; ++x) {
        for (int y = 0; y < grid_height; ++y) {
            auto& op = operators[get_operator_move_left_index(x, y)];

            op.name = "move-left <" + std::to_string(x) + "," +
                      std::to_string(y) + ">";
            op.cost = 1;
            op.precondition.emplace_back(get_fact_robot_at_square(x, y));
            op.effect.emplace_back(get_fact_robot_at_square(x - 1, y));
            op.effect.emplace_back(get_fact_square_visited(x - 1, y, true));
        }
    }

    // Up
    for (int x = 0; x < grid_width; ++x) {
        for (int y = 0; y < grid_height - 1; ++y) {
            auto& op = operators[get_operator_move_up_index(x, y)];
            op.name =
                "move-up <" + std::to_string(x) + "," + std::to_string(y) + ">";
            op.cost = 1;
            op.precondition.emplace_back(get_fact_robot_at_square(x, y));
            op.effect.emplace_back(get_fact_robot_at_square(x, y + 1));
            op.effect.emplace_back(get_fact_square_visited(x, y + 1, true));
        }
    }

    // Right
    for (int x = 0; x < grid_width - 1; ++x) {
        for (int y = 0; y < grid_height; ++y) {
            auto& op = operators[get_operator_move_right_index(x, y)];

            op.name = "move-right <" + std::to_string(x) + "," +
                      std::to_string(y) + ">";
            op.cost = 1;
            op.precondition.emplace_back(get_fact_robot_at_square(x, y));
            op.effect.emplace_back(get_fact_robot_at_square(x + 1, y));
            op.effect.emplace_back(get_fact_square_visited(x + 1, y, true));
        }
    }

    // Down
    for (int x = 0; x < grid_width; ++x) {
        for (int y = 1; y < grid_height; ++y) {
            auto& op = operators[get_operator_move_down_index(x, y)];

            op.name = "move-down <" + std::to_string(x) + "," +
                      std::to_string(y) + ">";
            op.cost = 1;
            op.precondition.emplace_back(get_fact_robot_at_square(x, y));
            op.effect.emplace_back(get_fact_robot_at_square(x, y - 1));
            op.effect.emplace_back(get_fact_square_visited(x, y - 1, true));
        }
    }
}

int VisitAll::get_variable_robot_at() const
{
    return 0;
}

int VisitAll::get_variable_square_visited(int x, int y) const
{
    verify_coordinate(x, y);
    return 1 + x * grid_height + y;
}

FactPair VisitAll::get_fact_robot_at_square(int x, int y) const
{
    verify_coordinate(x, y);
    return {get_variable_robot_at(), x * grid_height + y};
}

FactPair VisitAll::get_fact_square_visited(int x, int y, bool was_visited) const
{
    return {get_variable_square_visited(x, y), was_visited ? 1 : 0};
}

OperatorID VisitAll::get_operator_move_left_id(int x, int y) const
{
    return OperatorID(get_operator_move_left_index(x, y));
}

OperatorID VisitAll::get_operator_move_up_id(int x, int y) const
{
    return OperatorID(get_operator_move_up_index(x, y));
}

OperatorID VisitAll::get_operator_move_right_id(int x, int y) const
{
    return OperatorID(get_operator_move_right_index(x, y));
}

OperatorID VisitAll::get_operator_move_down_id(int x, int y) const
{
    return OperatorID(get_operator_move_down_index(x, y));
}

int VisitAll::get_operator_move_left_index(int x, int y) const
{
    verify_coordinate(x, y);
    verify_coordinate(x - 1, y);
    return (x - 1) * grid_height + y;
}

int VisitAll::get_operator_move_up_index(int x, int y) const
{
    verify_coordinate(x, y);
    verify_coordinate(x, y + 1);
    return (grid_width - 1) * grid_height + x * (grid_height - 1) + y;
}

int VisitAll::get_operator_move_right_index(int x, int y) const
{
    verify_coordinate(x, y);
    verify_coordinate(x + 1, y);
    return (grid_width - 1) * grid_height + grid_width * (grid_height - 1) + x * grid_height +
           y;
}

int VisitAll::get_operator_move_down_index(int x, int y) const
{
    verify_coordinate(x, y);
    verify_coordinate(x, y - 1);
    return 2 * (grid_width - 1) * grid_height + grid_width * (grid_height - 1) +
           x * (grid_height - 1) + (y - 1);
}

void VisitAll::verify_coordinate(int x, int y) const
{
    if (x < 0 || x >= grid_width || y < 0 || y >= grid_height) {
        throw std::range_error("Coordinate out of bounds!");
    }
}

} // namespace tests
