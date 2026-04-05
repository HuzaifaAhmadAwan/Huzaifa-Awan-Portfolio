#include "tests/domains/sokoban.h"

#include "downward/utils/system.h"

#include <set>

namespace tests {

namespace {
std::string to_string(std::pair<int, int> p)
{
    return "<" + std::to_string(p.first) + "," + std::to_string(p.second) + ">";
}
} // namespace

Sokoban::Sokoban(
    const std::vector<std::vector<char>>& grid,
    int num_boxes)
    : num_boxes(num_boxes)
{
    if (grid.empty()) {
        ABORT("Play area grid is empty!");
    }

    grid_width = grid[0].size();
    grid_height = grid.size();
    square_infos.resize(grid_width * grid_height);

    for (const auto& row : grid) {
        if (row.size() != static_cast<size_t>(grid_width)) {
            ABORT(
                "Play area defined with different row lengths!");
        }
    }

    int num_playable_sqs = 0;

    for (int j = 0; j != grid_height; ++j) {
        auto& row = grid[grid_height - 1 - j];
        for (int i = 0; i != grid_width; ++i) {
            auto g = row[i];
            auto& sq_info = square_infos[to_1D(i, j)];

            switch (g) {
            case ' ':
                sq_info.index = num_playable_sqs++;
                playable_squares.emplace_back(i, j);
                break;
            case 'G':
                sq_info.index = num_playable_sqs++;
                sq_info.is_goal = true;
                playable_squares.emplace_back(i, j);
                break;
            case '#': break;
            default: abort();
            }

            std::vector<int> vec(num_boxes, -1);

            sq_info.push_op_indices.emplace_back(vec);
            sq_info.push_op_indices.emplace_back(vec);
            sq_info.push_op_indices.emplace_back(vec);
            sq_info.push_op_indices.emplace_back(vec);
        }
    }

    auto to_string = [](std::pair<int, int> p) {
        return "(" + std::to_string(p.first) + "," + std::to_string(p.second) +
               ")";
    };

    variable_infos.resize(1 + 2 * num_boxes + playable_squares.size());

    // Variables
    {
        auto& v_info = variable_infos[get_variable_player_at()];

        v_info.name = "player-at";
        v_info.domain_size = num_playable_sqs;

        for (int i = 0; i != num_playable_sqs; ++i) {
            v_info.fact_names.emplace_back(
                "player-at " + to_string(index_to_square(i)));
        }
    }

    for (int b = 0; b != num_boxes; ++b) {
        auto& v_info = variable_infos[get_variable_box_at(b)];
        v_info.name = "box-" + std::to_string(b) + "-at";
        v_info.domain_size = num_playable_sqs;

        for (int i = 0; i != num_playable_sqs; ++i) {
            v_info.fact_names.emplace_back(
                "box-" + std::to_string(b) + "-at " +
                to_string(index_to_square(i)));
        }

        auto& v_info_goal = variable_infos[get_variable_box_at_goal(b)];

        v_info_goal.name = "box-" + std::to_string(b) + "-at-goal";
        v_info_goal.domain_size = 2;
        v_info_goal.fact_names.emplace_back(
            "box-" + std::to_string(b) + "-at-goal false");
        v_info_goal.fact_names.emplace_back(
            "box-" + std::to_string(b) + "-at-goal true");
    }

    for (int i = 0; i != num_playable_sqs; ++i) {
        const auto [sq_x, sq_y] = index_to_square(i);
        auto& v_info = variable_infos[get_variable_square_clear(sq_x, sq_y)];
        v_info.name =
            "square-clear-" + std::to_string(sq_x) + "-" + std::to_string(sq_y);
        v_info.domain_size = 2;
        v_info.fact_names.emplace_back(
            "square-clear-" + std::to_string(sq_x) + "-" +
            std::to_string(sq_y) + " false");
        v_info.fact_names.emplace_back(
            "square-clear-" + std::to_string(sq_x) + "-" +
            std::to_string(sq_y) + " true");
    }

    // Operators
    for (int j = 0; j != grid_height; ++j) {
        for (int i = 0; i != grid_width; ++i) {
            int square_index = square_to_index(i, j);
            auto& sq_info = square_infos[to_1D(i, j)];

            if (square_index == -1) continue;

            int left_x = i - 1;
            int right_x = i + 1;
            int up_y = j + 1;
            int down_y = j - 1;

            // Check left
            if (left_x >= 0 && left_x < grid_width &&
                square_to_index(left_x, j) != -1) {
                // Generate left move operator
                generate_move_op(sq_info, "left", 0, i, j, left_x, j);

                int left_left_x = i - 2;

                if (left_left_x >= 0 && left_left_x < grid_width &&
                    square_to_index(left_left_x, j) != -1) {
                    // Generate left push operators
                    bool is_goal = square_infos[to_1D(left_left_x, j)].is_goal;
                    generate_push_ops(
                        sq_info,
                        "left",
                        0,
                        i,
                        j,
                        left_x,
                        j,
                        left_left_x,
                        j,
                        is_goal);
                }
            }

            // Up
            if (up_y >= 0 && up_y < grid_height &&
                square_to_index(i, up_y) != -1) {
                // Generate up move operator
                generate_move_op(sq_info, "up", 1, i, j, i, up_y);

                int up_up_y = j + 2;

                if (up_up_y >= 0 && up_up_y < grid_height &&
                    square_to_index(i, up_up_y) != -1) {
                    // Generate up push operators
                    bool is_goal = square_infos[to_1D(i, up_up_y)].is_goal;
                    generate_push_ops(
                        sq_info,
                        "up",
                        1,
                        i,
                        j,
                        i,
                        up_y,
                        i,
                        up_up_y,
                        is_goal);
                }
            }

            // Right
            if (right_x >= 0 && right_x < grid_width &&
                square_to_index(right_x, j) != -1) {
                // Generate right move operator
                generate_move_op(sq_info, "right", 2, i, j, right_x, j);

                int right_right_x = i + 2;

                if (right_right_x >= 0 && right_right_x < grid_width &&
                    square_to_index(right_right_x, j) != -1) {
                    // Generate right push operators
                    bool is_goal =
                        square_infos[to_1D(right_right_x, j)].is_goal;
                    generate_push_ops(
                        sq_info,
                        "right",
                        2,
                        i,
                        j,
                        right_x,
                        j,
                        right_right_x,
                        j,
                        is_goal);
                }
            }

            // Down
            if (down_y >= 0 && down_y < grid_height &&
                square_to_index(i, down_y) != -1) {
                // Generate down move operator
                generate_move_op(sq_info, "down", 3, i, j, i, down_y);

                int down_down_y = j - 2;

                if (down_down_y >= 0 && down_down_y < grid_height &&
                    square_to_index(i, down_down_y) != -1) {
                    // Generate down push operators
                    bool is_goal = square_infos[to_1D(i, down_down_y)].is_goal;
                    generate_push_ops(
                        sq_info,
                        "down",
                        3,
                        i,
                        j,
                        i,
                        down_y,
                        i,
                        down_down_y,
                        is_goal);
                }
            }
        }
    }
}

int Sokoban::SquareInfo::get_left_move_index() const
{
    return move_op_indices[0];
}

int Sokoban::SquareInfo::get_up_move_index() const
{
    return move_op_indices[1];
}

int Sokoban::SquareInfo::get_right_move_index() const
{
    return move_op_indices[2];
}

int Sokoban::SquareInfo::get_down_move_index() const
{
    return move_op_indices[3];
}

int Sokoban::SquareInfo::get_left_push_index(int crate) const
{
    return push_op_indices[0][crate];
}

int Sokoban::SquareInfo::get_up_push_index(int crate) const
{
    return push_op_indices[1][crate];
}

int Sokoban::SquareInfo::get_right_push_index(int crate) const
{
    return push_op_indices[2][crate];
}

int Sokoban::SquareInfo::get_down_push_index(int crate) const
{
    return push_op_indices[3][crate];
}

int Sokoban::to_1D(int x, int y) const
{
    return x + grid_width * y;
}

int Sokoban::square_to_index(int x, int y) const
{
    return square_infos[to_1D(x, y)].index;
}

std::pair<int, int> Sokoban::index_to_square(int index) const
{
    return playable_squares[index];
}

int Sokoban::get_variable_player_at() const
{
    return 0;
}

int Sokoban::get_variable_box_at(int box_idx) const
{
    return 1 + box_idx;
}

int Sokoban::get_variable_box_at_goal(int box_id) const
{
    return 1 + num_boxes + box_id;
}

int Sokoban::get_variable_square_clear(int x, int y) const
{
    verify_square(x, y);
    return 1 + 2 * num_boxes + square_to_index(x, y);
}

FactPair Sokoban::get_fact_player_at(int x, int y) const
{
    verify_square(x, y);
    return {get_variable_player_at(), square_to_index(x, y)};
}

FactPair Sokoban::get_fact_box_at(int box_idx, int x, int y) const
{
    verify_square(x, y);
    return {get_variable_box_at(box_idx), square_to_index(x, y)};
}

FactPair Sokoban::get_fact_box_at_goal(int box_idx, bool is_at_goal) const
{
    return {get_variable_box_at_goal(box_idx), is_at_goal ? 1 : 0};
}

FactPair Sokoban::get_fact_square_clear(int x, int y, bool is_clear) const
{
    return {get_variable_square_clear(x, y), is_clear ? 1 : 0};
}

OperatorID Sokoban::get_operator_move_left_id(int p_x, int p_y) const
{
    return OperatorID(get_operator_move_left_index(p_x, p_y));
}

OperatorID Sokoban::get_operator_move_up_id(int p_x, int p_y) const
{
    return OperatorID(get_operator_move_up_index(p_x, p_y));
}

OperatorID Sokoban::get_operator_move_right_id(int p_x, int p_y) const
{
    return OperatorID(get_operator_move_right_index(p_x, p_y));
}

OperatorID Sokoban::get_operator_move_down_id(int p_x, int p_y) const
{
    return OperatorID(get_operator_move_down_index(p_x, p_y));
}

OperatorID Sokoban::get_operator_push_left_id(int crate, int p_x, int p_y) const
{
    return OperatorID(get_operator_push_left_index(crate, p_x, p_y));
}

OperatorID Sokoban::get_operator_push_up_id(int crate, int p_x, int p_y) const
{
    return OperatorID(get_operator_push_up_index(crate, p_x, p_y));
}

OperatorID
Sokoban::get_operator_push_right_id(int crate, int p_x, int p_y) const
{
    return OperatorID(get_operator_push_right_index(crate, p_x, p_y));
}

OperatorID Sokoban::get_operator_push_down_id(int crate, int p_x, int p_y) const
{
    return OperatorID(get_operator_push_down_index(crate, p_x, p_y));
}

int Sokoban::get_operator_move_left_index(int p_x, int p_y) const
{
    return square_infos[to_1D(p_x, p_y)].get_left_move_index();
}

int Sokoban::get_operator_move_up_index(int p_x, int p_y) const
{
    return square_infos[to_1D(p_x, p_y)].get_up_move_index();
}

int Sokoban::get_operator_move_right_index(int p_x, int p_y) const
{
    return square_infos[to_1D(p_x, p_y)].get_right_move_index();
}

int Sokoban::get_operator_move_down_index(int p_x, int p_y) const
{
    return square_infos[to_1D(p_x, p_y)].get_down_move_index();
}

int Sokoban::get_operator_push_left_index(int crate, int p_x, int p_y)
    const
{
    return square_infos[to_1D(p_x, p_y)].get_left_push_index(crate);
}

int Sokoban::get_operator_push_up_index(int crate, int p_x, int p_y)
    const
{
    return square_infos[to_1D(p_x, p_y)].get_up_push_index(crate);
}

int Sokoban::get_operator_push_right_index(int crate, int p_x, int p_y)
    const
{
    return square_infos[to_1D(p_x, p_y)].get_right_push_index(crate);
}

int Sokoban::get_operator_push_down_index(int crate, int p_x, int p_y)
    const
{
    return square_infos[to_1D(p_x, p_y)].get_down_push_index(crate);
}

void Sokoban::verify_square(int x, int y) const
{
    if (x < 0 || x >= grid_width || y < 0 || y >= grid_height) {
        ABORT("Square out of bounds!");
    }

    if (square_to_index(x, y) == -1) {
        ABORT("Square in bounds, but not inside the play area!");
    }
}

void Sokoban::generate_move_op(
    SquareInfo& sq_info,
    std::string name,
    int dir_index,
    int p_x,
    int p_y,
    int dest_x,
    int dest_y)
{
    sq_info.move_op_indices[dir_index] = operators.size();

    auto& mve = operators.emplace_back();
    mve.name = "move-" + name + " " + to_string(std::make_pair(p_x, p_y));
    mve.cost = 1;

    mve.precondition.emplace_back(get_fact_player_at(p_x, p_y));
    mve.precondition.emplace_back(get_fact_square_clear(dest_x, dest_y, true));

    mve.effect.emplace_back(get_fact_square_clear(p_x, p_y, true));
    mve.effect.emplace_back(get_fact_player_at(dest_x, dest_y));
    mve.effect.emplace_back(get_fact_square_clear(dest_x, dest_y, false));
}

void Sokoban::generate_push_ops(
    SquareInfo& sq_info,
    std::string name,
    int dir_index,
    int p_x,
    int p_y,
    int box_x,
    int box_y,
    int dest_x,
    int dest_y,
    bool goal_dest)
{
    for (int c = 0; c != num_boxes; ++c) {
        sq_info.push_op_indices[dir_index][c] = operators.size();

        auto& push = operators.emplace_back();

        push.name = "push-" + name + " " + std::to_string(c) + " " +
                    to_string(std::make_pair(p_x, p_y));
        push.cost = 1;

        push.precondition.emplace_back(get_fact_player_at(p_x, p_y));
        push.precondition.emplace_back(get_fact_box_at(c, box_x, box_y));
        push.precondition.emplace_back(
            get_fact_square_clear(dest_x, dest_y, true));

        push.effect.emplace_back(get_fact_square_clear(p_x, p_y, true));
        push.effect.emplace_back(get_fact_square_clear(dest_x, dest_y, false));
        push.effect.emplace_back(get_fact_player_at(box_x, box_y));
        push.effect.emplace_back(get_fact_box_at(c, dest_x, dest_y));
        push.effect.emplace_back(get_fact_box_at_goal(c, goal_dest));
    }
}

} // namespace tests
