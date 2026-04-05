#include "tests/utils/ff_utils.h"

#include "downward/heuristics/best_supporter_function.h"

#include "downward/fact_enumerator.h"
#include "downward/state.h"
#include "downward/task_proxy.h"

#include "tests/utils/input_utils.h"

#include <filesystem>
#include <fstream>
#include <set>

namespace {

template <typename Facts>
bool is_subset_of(const Facts& left, const std::set<FactPair>& right)
{
    for (FactProxy precondition : left) {
        if (!right.contains(precondition.get_pair())) return false;
    }
    return true;
}

class ListBestSupporterFunction : public BestSupporterFunction {
    const FactEnumerator enumerator;
    std::vector<std::optional<OperatorID>> best_supporters;

public:
    ListBestSupporterFunction(
        const ClassicalPlanningTask& task,
        const std::filesystem::path& filepath)
        : enumerator(task.get_variables())
        , best_supporters(enumerator.get_num_facts(), std::nullopt)
    {
        std::ifstream is(filepath);

        for (is >> std::ws; is.peek() != EOF; is >> std::ws) {
            assert(std::isdigit(is.peek()));

            FactPair fact = FactPair::no_fact;
            is >> fact.var;
            assert(is.peek() == '=');
            is.ignore(1);
            is >> fact.value;

            is >> std::ws;

            assert(is.peek() == ':');
            is.ignore(1);

            is >> std::ws;

            assert(std::isdigit(is.peek()));
            int op_id;
            is >> op_id;

            is >> std::ws;

            best_supporters[enumerator.get_fact_index(fact)] =
                OperatorID(op_id);

            if (is.peek() == ',') is.ignore(1);
        }
    }

    void compute_best_supporters(const State&) override {}

    std::optional<OperatorID> get_best_supporter(const FactPair& fact) override
    {
        return best_supporters[enumerator.get_fact_index(fact)];
    }
};

} // namespace

namespace tests {

std::optional<DeleteRelaxedPlan> get_relaxed_plan(
    const ClassicalPlanningTask& task,
    DeleteRelaxedPlanHeuristic& heuristic)
{
    return heuristic.compute_relaxed_plan(task.get_initial_state());
}

bool verify_relaxed_plan(
    const ClassicalPlanningTask& task,
    const std::vector<OperatorID>& relaxed_plan,
    bool reject_superfluous_operators)
{
    OperatorsProxy operators = task.get_operators();

    State initial_state = task.get_initial_state();

    std::set<FactPair> facts;

    for (std::size_t i = 0; i != initial_state.size(); ++i) {
        facts.emplace(i, initial_state[i]);
    }

    for (OperatorID op_id : relaxed_plan) {
        if (reject_superfluous_operators &&
            is_subset_of(task.get_goal(), facts))
            return false;

        OperatorProxy op = operators[op_id];

        if (!is_subset_of(op.get_preconditions(), facts)) return false;

        for (auto fact : op.get_effects()) {
            facts.insert(fact.get_pair());
        }
    }

    return is_subset_of(task.get_goal(), facts);
}

std::shared_ptr<BestSupporterFunction> read_best_supporter_function_from_file(
    const ClassicalPlanningTask& task,
    const std::string& filename)
{
    const std::filesystem::path filepath =
        RESOURCES_PATH / "best_supporter_functions" / filename;
    return std::make_shared<ListBestSupporterFunction>(task, filepath);
}

void dump_best_supporters_to_file(
    const ClassicalPlanningTask& task,
    BestSupporterFunction& bsf,
    const std::string& filename)
{
    const std::filesystem::path filepath =
        RESOURCES_PATH / "best_supporter_functions" / filename;
    std::ofstream file(filepath);

    FactsProxy facts = task.get_facts();
    State init_state = task.get_initial_state();
    bsf.compute_best_supporters(init_state);

    auto it = facts.begin();
    auto end = facts.end();

    for (; it != end; ++it) {
        auto fact = *it;
        auto bs = bsf.get_best_supporter(fact.get_pair());
        if (bs.has_value()) {
            std::string bs_text = std::to_string(bs->get_index());
            file << fact.get_pair() << " : " << bs_text;
            for (++it; it != end; ++it) {
                auto fact2 = *it;
                auto bs2 = bsf.get_best_supporter(fact2.get_pair());
                if (!bs2.has_value()) continue;
                std::string bs_text2 = std::to_string(bs2->get_index());
                file << ", " << fact2.get_pair() << " : " << bs_text2;
            }
            return;
        }
    }
}

} // namespace tests
