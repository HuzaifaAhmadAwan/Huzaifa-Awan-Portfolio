#ifndef TESTS_DOMAINS_SOKOBAN_H
#define TESTS_DOMAINS_SOKOBAN_H

#include "tests/domains/classical_planning_domain.h"

#include "downward/task_proxy.h"

#include <vector>

namespace tests {

// A matrix of characters (see constructor of Sokoban)
using SokobanGrid = std::vector<std::vector<char>>;

/**
 * @brief Models the puzzle planning domain BlocksWorld.
 *
 * Sokoban is a single-player puzzle game taking place in a 2D grid with fixed
 * width and height with walls and crates (link to a web version:
 * https://www.mathsisfun.com/games/sokoban.html). In the following, we denote
 * the set of coordinates in the grid that are not inhabited by a wall as the
 * "play area". The task of this game is to push all crates into designated goal
 * positions within the play area. As soon as every crate is on a goal position,
 * the puzzle is solved. There are two kinds of actions the player character can
 * do:
 *
 * 1. The player character may move from one square in the play area to another
 * horizontally or vertically, but not diagonally adjacent square inside the
 * play area, unless the square is blocked by a crate.
 * 2. The player character can push a horizontally or vertically, but not
 * diagonally adjacent crate forward by one square, unless the square behind the
 * crate is blocked by another crate or not inside the playarea. Doing so also
 * moves the player character forward by one square in the same direction.
 *
 * Notably, a crate cannot be dragged sideways or pulled, which produces
 * situations in which the puzzle becomes unsolvable.
 *
 * This environment is modelled as an FDR task as follows:
 *
 * Variables:
 * + Player-At - position of the player, a 2D coordinate inside the play area.
 * + Crate-At(c) - position of a crate c, a 2D coordinate inside the play area.
 * + Crate-At-Goal(c) - whether a crate c is on some goal square. True or false.
 * + Square-Clear(x, y) - whether the play area position <x, y> is occupied by a
 * crate. True or false.
 *
 * Here, we assume "image coordinates", i.e., the x-axis goes from left-to-right
 * and the y-axis goes from top to bottom for the 2D grid coordinates. The
 * coordinate <0, 0> therefore designates the top-left corner of the grid.
 *
 * Operators:
 * + Move-Left(x, y) - The player character moves left by one square.
 *   Only exists if the square <x-1, y> is inside the play area.
 *   - Precondition: Player-At=<x, y>, Square-Clear(x-1, y)=true
 *   - Effect: Player-At=<x-1, y>
 * + Move-Up(x, y) - The player character moves up by one square.
 *   Only exists if the square <x, y-1> is inside the play area.
 *   - Precondition: Player-At=<x, y>, Square-Clear(x, y-1)=true
 *   - Effect: Player-At=<x, y-1>
 * + Move-Right(x, y) - The player character moves right by one square.
 *   Only exists if the square <x+1, y> is inside the play area.
 *   - Precondition: Player-At=<x, y>, Square-Clear(x+1, y)=true
 *   - Effect: Player-At=<x+1, y>
 * + Move-Down(x, y) - The player character moves down by one square.
 *   Only exists if the square <x, y+1> is inside the play area.
 *   - Precondition: Player-At=<x, y>, Square-Clear(x, y+1)=true
 *   - Effect: Player-At=<x, y+1>
 * + Push-Left(c, x, y) - The player character pushes a crate c to his left by
 *   one square. Only exists if the squares <x-1, y> and <x-2, y> are inside the
 *   play area.
 *   - Precondition: Player-At=<x, y>, Crate-At(c)=<x-1, y>,
 *                   Square-Clear(x-2, y)=true
 *   - Effect: Player-At=<x-1, y>, Crate-At(c)=<x-2, y>,
 *             Square-Clear(x-2, y)=false,
 *             Crate-At-Goal(c)= | true   if <x-2, y> is a goal position
 *                               | false  otherwise
 * + Push-Up(c, x, y) - The player character pushes a crate c above him by one
 *   square. Only exists if the squares <x, y-1> and <x, y-2> are inside the
 *   play area.
 *   - Precondition: Player-At=<x, y>, Crate-At(c)=<x, y-1>,
 *                   Square-Clear(x, y-2)=true
 *   - Effect: Player-At=<x, y-1>, Crate-At(c)=<x, y-1>,
 *             Square-Clear(x, y-2)=false,
 *             Crate-At-Goal(c)= | true   if <x, y-2> is a goal position
 *                               | false  otherwise
 * + Push-Right(c, x, y) - The player character pushes a crate c to his right by
 *   one square. Only exists if the squares <x+1, y> and <x+2, y> are inside the
 *   play area.
 *   - Precondition: Player-At=<x, y>, Crate-At(c)=<x+1, y>,
 *                   Square-Clear(x+2, y)=true
 *   - Effect: Player-At=<x+1, y>, Crate-At(c)=<x+2, y>,
 *             Square-Clear(x+2, y)=false,
 *             Crate-At-Goal(c)= | true   if <x+2, y> is a goal position
 *                               | false  otherwise
 * + Push-Down(c, x, y) - The player character pushes a crate c below him by one
 *   square. Only exists if the squares <x, y+1> and <x, y+2> are inside the
 *   play area.
 *   - Precondition: Player-At=<x, y>, Crate-At(c)=<x, y+1>,
 *                   Square-Clear(x, y+2)=true
 *   - Effect: Player-At=<x, y+1>, Crate-At(c)=<x, y+2>,
 *             Square-Clear(x, y+2)=false,
 *             Crate-At-Goal(c)= | true   if <x, y+2> is a goal position
 *                               | false  otherwise
 *
 * All operators have unit costs.
 *
 * @ingroup test_domains
 */
class Sokoban : public ClassicalPlanningDomain {
    struct SquareInfo {
        int index = -1;
        bool is_goal = false;
        std::vector<int> move_op_indices = {-1, -1, -1, -1};
        std::vector<std::vector<int>> push_op_indices;

        int get_left_move_index() const;
        int get_up_move_index() const;
        int get_right_move_index() const;
        int get_down_move_index() const;

        int get_left_push_index(int crate) const;
        int get_up_push_index(int crate) const;
        int get_right_push_index(int crate) const;
        int get_down_push_index(int crate) const;
    };

    int num_boxes;
    int grid_width;
    int grid_height;
    std::vector<SquareInfo> square_infos;
    std::vector<std::pair<int, int>> playable_squares;

public:
    /**
     * Construct a Sokoban puzzle from a 2D array specifying the play area and
     * the goal positions. The following representation is used:
     * - ' ' (space) denotes a free field
     * - '#' denotes a wall
     * - 'G' denotes a goal position
     *
     * Example:
     *
     * const SokobanGrid playarea = {
     *   {'#', '#', 'G'},
     *   {' ', ' ', 'G'}
     *   {' ', ' ', ' '}
     * };
     *
     * corresponds to
     *
     *                   |========|
     *                   |        |
     *                   |  Goal  |
     *                   |        |
     * |==========================|
     * |        |        |        |
     * |        |        |  Goal  |
     * |        |        |        |
     * |==========================|
     * |        |        |        |
     * |        |        |        |
     * |        |        |        |
     * |==========================|
     *
     * Remark: playarea[x][y] corresponds to position <y, x> in the grid. It was
     * implemented this way to keep the visual correspondence between the
     * intended play area and the one declared using aggregate initialization
     * syntax, as in the above example.
     */
    Sokoban(const SokobanGrid& grid, int num_boxes);

    // Get the integer representation of a variable.
    int get_variable_player_at() const;
    int get_variable_box_at(int box_idx) const;
    int get_variable_box_at_goal(int box_id) const;
    int get_variable_square_clear(int x, int y) const;

    // Get the FactPair representation of a fact.
    FactPair get_fact_player_at(int x, int y) const;
    FactPair get_fact_box_at(int box_idx, int x, int y) const;
    FactPair get_fact_box_at_goal(int box_idx, bool is_at_goal) const;
    FactPair get_fact_square_clear(int x, int y, bool is_clear) const;

    // Get the OperatorID representation of an operator.
    OperatorID get_operator_move_left_id(int p_x, int p_y) const;
    OperatorID get_operator_move_up_id(int p_x, int p_y) const;
    OperatorID get_operator_move_right_id(int p_x, int p_y) const;
    OperatorID get_operator_move_down_id(int p_x, int p_y) const;

    OperatorID get_operator_push_left_id(int crate, int p_x, int p_y) const;
    OperatorID get_operator_push_up_id(int crate, int p_x, int p_y) const;
    OperatorID get_operator_push_right_id(int crate, int p_x, int p_y) const;
    OperatorID get_operator_push_down_id(int crate, int p_x, int p_y) const;

private:
    Sokoban(const char** grid, int num_boxes, int M, int N);

    int to_1D(int x, int y) const;
    int square_to_index(int x, int y) const;
    std::pair<int, int> index_to_square(int index) const;

    int get_operator_move_left_index(int p_x, int p_y) const;
    int get_operator_move_up_index(int p_x, int p_y) const;
    int get_operator_move_right_index(int p_x, int p_y) const;
    int get_operator_move_down_index(int p_x, int p_y) const;

    int get_operator_push_left_index(int crate, int p_x, int p_y) const;
    int get_operator_push_up_index(int crate, int p_x, int p_y) const;
    int get_operator_push_right_index(int crate, int p_x, int p_y) const;
    int get_operator_push_down_index(int crate, int p_x, int p_y) const;

    void verify_square(int x, int y) const;

    void generate_move_op(
        SquareInfo& sq_info,
        std::string name,
        int dir_index,
        int p_x,
        int p_y,
        int dest_x,
        int dest_y);

    void generate_push_ops(
        SquareInfo& sq_info,
        std::string name,
        int dir_index,
        int p_x,
        int p_y,
        int box_x,
        int box_y,
        int dest_x,
        int dest_y,
        bool goal_dest);
};

} // namespace tests

#endif