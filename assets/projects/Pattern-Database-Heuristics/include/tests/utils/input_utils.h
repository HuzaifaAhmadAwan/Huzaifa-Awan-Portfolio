#ifndef UTILS_INPUT_UTILS_H
#define UTILS_INPUT_UTILS_H

#include "downward/task_proxy.h"

#include <filesystem>
#include <iosfwd>
#include <memory>
#include <source_location>
#include <string_view>

class ClassicalPlanningTask;
class OperatorID;

namespace probfd {
class ProbabilisticPlanningTask;
} // namespace probfd

namespace tests {

/// Absolute path to the resources directory of the project.
inline const std::filesystem::path RESOURCES_PATH =
    std::filesystem::path(std::source_location::current().file_name())
        .parent_path()
        .parent_path()
        .parent_path()
        .parent_path() /
    "resources";

/// Exception thrown when a file was not found.
class FileNotFoundError : public std::runtime_error {
public:
    explicit FileNotFoundError(const std::string& file_path = "")
        : runtime_error("cannot find file " + file_path)
    {
    }
};

// clang-format off
/**
 * @brief Reads a task from a `.sas` file with the given filename in the
 * `resources/problems` directory of the project.
 *
 * @see read_task_from_file
 *
 * @ingroup classical_planning_utils
 */
// clang-format on
std::unique_ptr<ClassicalPlanningTask>
read_task_from_resources_file(std::string_view task_filename);

// clang-format off
/**
 * @brief Reads a task from a file at the given file system path.
 *
 * To generate a `.sas` problem file from one of the PDDL problems in the
 * `domains` directory, invoke the PDDL translator as follows:
 *
 * ```sh
 * ./fast-downward.py --sas-file resources/problems/my_problem.sas domains/some_domain/some_problem.pddl
 * ```
 *
 * This writes a translated `.sas` file for the problem to
 * `resources/problems/my_problem.sas`.
 * The task can then be loaded from the file with:
 * ```cpp
 * auto task = tests::read_task_from_file("resources/problems/my_problem.sas");
 * ```
 * or even easier via
 * ```cpp
 * auto task = tests::read_task_from_resources_file("my_problem.sas");
 * ```
 *
 * @ingroup classical_planning_utils
 */
// clang-format on
std::unique_ptr<ClassicalPlanningTask>
read_task_from_file(const std::filesystem::path& path);

// clang-format off
/**
 * @brief Reads a plan for the specified task from a `.plan` file with the given
 * filename located in the `resources/plans` directory of the project.
 *
 * @see read_plan_from_file
 *
 * @ingroup classical_planning_utils
 */
// clang-format on
std::vector<OperatorID> read_plan_from_resources_file(
    const ClassicalPlanningTask&task,
    std::string_view plan_filename);

// clang-format off
/**
 * @brief Reads a plan for the specified task from a file at the given file
 * system path.
 *
 * Roughly equivalent to:
 *
 * ```sh
 * std::ifstream file(filepath);
 * std::vector<int> plan = read_plan(file);
 * ```
 * 
 * @see read_plan
 * 
 * @ingroup test_utils
 */
// clang-format on
std::vector<OperatorID> read_plan_from_file(
    const ClassicalPlanningTask& task,
    const std::filesystem::path& filepath);

/**
 * @brief Reads a plan for the specified task from an input stream.
 *
 * The plan format is as follows:
 *
 * ```txt
 * (<first-operator-name>)
 * (<second-operator-name)
 * ...
 * (<final-operator-name>)
 * ; <optional plan cost>
 * ```
 *
 * Every operator name is parenthesized and is stated on a seperate line.
 * No gaps between operators may exist.
 * The plan ends with a semicolon on a seperate line.
 * The plan cost is ignored and optional.
 *
 * To generate an optimal plan for one of the PDDL problems in the
 * `domains` directory, invoke the integrated optimal planner as follows:
 *
 * ```sh
 * # hmax() is also possible instead of blind()
 * ./fast-downward.py --plan-file resources/plans/my_plan.plan
 * domains/some_domain/some_problem.pddl --search "astar(blind())"
 * ```
 *
 * This runs A* search with the specified heuristic to find an optimal plan for
 * the problem and writes it to a file `resources/plans/my_plan.plan`.
 *
 * @ingroup classical_planning_utils
 *
 * @attention
 * If you generate a plan file by invoking the planner, you need to have a
 * correct implementation of the heuristics you use in the search algorithm
 * configuration to compute the plan!
 * If your implementation is incorrect, the emitted plan (if the planner does
 * not crash) may differ from the plan that a correct implementation would
 * produce.
 */
 // clang-format on
std::vector<OperatorID> read_plan(
    const ClassicalPlanningTask&task,
    std::istream& filepath);

/**
 * @brief Reads a list of integers from a file specified by file system path.
 *
 * @see read_list
 *
 * @ingroup classical_planning_utils
 */
std::vector<int> read_list_from_file(const std::filesystem::path& filepath);

/**
 * @brief Reads a list of integers from an input stream.
 *
 * @ingroup test_utils
 */
std::vector<int> read_list(std::istream& is);

/**
 * @brief Write a list of integers to a file at the given file system path.
 *
 * @see write_list
 *
 * @ingroup test_utils
 */
void write_list_to_file(
    const std::vector<int>& list,
    const std::filesystem::path& filepath);

/**
 * @brief Write a list of integers to an output stream.
 *
 * @ingroup test_utils
 */
void write_list(const std::vector<int>& list, std::ostream& out);

/**
 * @brief Reads a list of lists of integers from a file specified by file system
 * path.
 *
 * @see read_lists
 *
 * @ingroup test_utils
 */
std::vector<std::vector<int>> read_lists_from_file(
    const std::filesystem::path& filepath,
    char delimiter = ';');

/**
 * @brief Reads a list of lists of integers from an input stream.
 *
 * @ingroup test_utils
 */
std::vector<std::vector<int>>
read_lists(std::istream& is, char delimiter = ';');

/**
 * @brief Write a list of lists of integers to a file at the given file system
 * path.
 *
 * @see write_lists
 *
 * @ingroup test_utils
 */
void write_lists_to_file(
    const std::vector<std::vector<int>>& lists,
    const std::filesystem::path& filepath,
    char delimiter = ';');

/**
 * @brief Write a list of lists of integers to an output stream.
 *
 * @ingroup test_utils
 */
void write_lists(
    const std::vector<std::vector<int>>& lists,
    std::ostream& os,
    char delimiter = ';');

} // namespace tests

#endif // UTILS_INPUT_UTILS_H
