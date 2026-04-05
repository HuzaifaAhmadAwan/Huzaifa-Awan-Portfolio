#include "tests/utils/input_utils.h"

#include "downward/tasks/root_task.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>

using namespace std;

namespace tests {

static const filesystem::path PROBLEMS_PATH = RESOURCES_PATH / "problems";
static const filesystem::path PLANS_PATH = RESOURCES_PATH / "plans";

static void check_file_exists(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        throw FileNotFoundError(path.string());
    }
}

std::unique_ptr<ClassicalPlanningTask>
read_task_from_resources_file(std::string_view task_filename)
{
    return read_task_from_file(PROBLEMS_PATH / task_filename);
}

std::unique_ptr<ClassicalPlanningTask>
read_task_from_file(const std::filesystem::path& path)
{
    check_file_exists(path);

    std::fstream problem_file(path);
    return tasks::read_task_from_sas(problem_file);
}

std::vector<OperatorID> read_plan_from_resources_file(
    const ClassicalPlanningTask& task_proxy,
    std::string_view plan_filename)
{
    return read_plan_from_file(task_proxy, PLANS_PATH / plan_filename);
}

std::vector<OperatorID> read_plan_from_file(
    const ClassicalPlanningTask& task_proxy,
    const std::filesystem::path& path)
{
    check_file_exists(path);

    std::fstream file(path);
    return read_plan(task_proxy, file);
}

std::vector<OperatorID> read_plan(
    const ClassicalPlanningTask& task_proxy,
    std::istream& in)
{
    std::vector<OperatorID> plan;

    for (std::string line; std::getline(in, line) && line[0] != ';';) {
        std::string operator_name = line.substr(1, line.size() - 2);

        for (OperatorProxy op : task_proxy.get_operators()) {
            if (op.get_name() == operator_name) {
                plan.emplace_back(op.get_id());
            }
        }
    }

    return plan;
}

std::vector<int> read_list_from_file(const std::filesystem::path& filepath)
{
    check_file_exists(filepath);

    std::ifstream file(filepath);

    std::vector<int> list;

    int value;

    while (file >> value) {
        list.push_back(value);
    }

    return list;
}

void write_list_to_file(
    const std::vector<int>& list,
    const std::filesystem::path& filepath)
{
    std::filesystem::create_directories(filepath.parent_path());

    std::ofstream file(filepath);

    if (!file) {
        std::cerr << "Could not open file: " << filepath;
        abort();
    }

    if (list.empty()) return;

    file << list.front();
    for (int value : list | std::views::drop(1)) {
        file << " " << value;
    }
}

} // namespace tests