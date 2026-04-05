#include "tests/domains/blocksworld.h"

#include "downward/utils/system.h"

#include <set>

namespace tests {

BlocksWorld::BlocksWorld(
    int num_blocks,
    int pick_cost,
    int put_cost)
    : ClassicalPlanningDomain(2 * num_blocks + 1, 2 * num_blocks * (num_blocks + 1))
    , num_blocks(num_blocks)
{
    variable_infos[get_variable_hand_empty()] = VariableInfo(
        "is-hand-empty",
        2,
        std::vector<std::string>({"hand-not-empty", "hand-empty"}));

    for (int b = 0; b != num_blocks; ++b) {
        std::vector<std::string> fact_names;

        for (int b2 = 0; b2 != num_blocks; ++b2) {
            fact_names.emplace_back(
                "block-" + std::to_string(b) + "-on-" + std::to_string(b2));
        }

        fact_names.emplace_back("block-" + std::to_string(b) + "-on-table");
        fact_names.emplace_back("block-" + std::to_string(b) + "-in-hand");

        variable_infos[get_variable_location(b)] = VariableInfo(
            "b" + std::to_string(b) + "-on",
            num_blocks + 2,
            std::move(fact_names));
    }

    for (int b = 0; b != num_blocks; ++b) {
        std::vector<std::string> fact_names;

        for (int b2 = 0; b2 != num_blocks; ++b2) {
            fact_names.emplace_back(
                "block-" + std::to_string(b) + "-on-" + std::to_string(b2));
        }

        fact_names.emplace_back("block-" + std::to_string(b) + "-on-table");
        fact_names.emplace_back("block-" + std::to_string(b) + "-in-hand");

        variable_infos[get_variable_clear(b)] = VariableInfo(
            "is-clear-b" + std::to_string(b),
            2,
            std::vector<std::string>(
                {"block-" + std::to_string(b) + "-not-clear",
                 "block-" + std::to_string(b) + "-clear"}));
    }

    for (int b1 = 0; b1 != num_blocks; ++b1) {
        for (int b2 = 0; b2 != num_blocks; ++b2) {
            operators[get_operator_index_pick_from_block(b1, b2)] =
                OperatorInfo(
                    "pick-up-b" + std::to_string(b1) + "-b" +
                        std::to_string(b2),
                    pick_cost,
                    std::vector<FactPair>{
                        get_fact_is_hand_empty(true),
                        get_fact_location_on_block(b1, b2),
                        get_fact_is_clear(b1, true)},
                    std::vector<FactPair>{
                        get_fact_is_hand_empty(false),
                        get_fact_location_in_hand(b1),
                        get_fact_is_clear(b1, false),
                        get_fact_is_clear(b2, true)});
        }
    }

    for (int b = 0; b != num_blocks; ++b) {
        operators[get_operator_index_pick_from_table(b)] = OperatorInfo(
            "pick-up-from-table-b" + std::to_string(b),
            pick_cost,
            std::vector<FactPair>{
                get_fact_is_hand_empty(true),
                get_fact_location_on_table(b),
                get_fact_is_clear(b, true)},
            std::vector<FactPair>{
                get_fact_is_hand_empty(false),
                get_fact_location_in_hand(b),
                get_fact_is_clear(b, false)});
    }

    for (int b1 = 0; b1 != num_blocks; ++b1) {
        for (int b2 = 0; b2 != num_blocks; ++b2) {
            operators[get_operator_index_put_on_block(b1, b2)] = OperatorInfo(
                "put-on-block-b" + std::to_string(b1) + "-b" +
                    std::to_string(b2),
                put_cost,
                std::vector<FactPair>{
                    get_fact_location_in_hand(b1),
                    get_fact_is_clear(b2, true)},
                std::vector<FactPair>{
                    get_fact_is_hand_empty(true),
                    get_fact_location_on_block(b1, b2),
                    get_fact_is_clear(b1, true),
                    get_fact_is_clear(b2, false)});
        }
    }

    for (int b = 0; b != num_blocks; ++b) {
        operators[get_operator_index_put_on_table(b)] = OperatorInfo(
            "put-on-table-b" + std::to_string(b),
            put_cost,
            std::vector<FactPair>{get_fact_location_in_hand(b)},
            std::vector<FactPair>{
                get_fact_is_hand_empty(true),
                get_fact_location_on_table(b),
                get_fact_is_clear(b, true)});
    }
}

int BlocksWorld::get_variable_hand_empty() const
{
    return 0;
}

int BlocksWorld::get_variable_location(int block) const
{
    verify_block_index(block);
    return 1 + block;
}

int BlocksWorld::get_variable_clear(int block) const
{
    verify_block_index(block);
    return 1 + num_blocks + block;
}

FactPair BlocksWorld::get_fact_is_clear(int block, bool is_clear) const
{
    return {get_variable_clear(block), is_clear ? 1 : 0};
}

FactPair
BlocksWorld::get_fact_location_on_block(int block, int under_block) const
{
    verify_block_index(under_block);
    return {get_variable_location(block), under_block};
}

FactPair BlocksWorld::get_fact_location_on_table(int block) const
{
    return {get_variable_location(block), num_blocks};
}

FactPair BlocksWorld::get_fact_location_in_hand(int block) const
{
    return {get_variable_location(block), num_blocks + 1};
}

FactPair BlocksWorld::get_fact_is_hand_empty(bool is_empty) const
{
    return {get_variable_hand_empty(), is_empty ? 1 : 0};
}

OperatorID BlocksWorld::get_operator_id_pick_from_block(int b1, int b2) const
{
    return OperatorID(get_operator_index_pick_from_block(b1, b2));
}

OperatorID BlocksWorld::get_operator_id_pick_from_table(int b) const
{
    return OperatorID(get_operator_index_pick_from_table(b));
}

OperatorID BlocksWorld::get_operator_id_put_on_block(int b1, int b2) const
{
    return OperatorID(get_operator_index_put_on_block(b1, b2));
}

OperatorID BlocksWorld::get_operator_id_put_on_table(int b) const
{
    return OperatorID(get_operator_index_put_on_table(b));
}

int BlocksWorld::get_operator_index_pick_from_block(int b1, int b2) const
{
    return b1 * num_blocks + b2;
}

int BlocksWorld::get_operator_index_pick_from_table(int b) const
{
    return num_blocks * num_blocks + b;
}

int BlocksWorld::get_operator_index_put_on_block(int b1, int b2) const
{
    return num_blocks * num_blocks + num_blocks + b1 * num_blocks + b2;
}

int BlocksWorld::get_operator_index_put_on_table(int b) const
{
    return num_blocks * num_blocks + num_blocks + num_blocks * num_blocks + b;
}

void BlocksWorld::verify_block_index(int block) const
{
    if (block < 0 || block >= num_blocks) {
        ABORT("Usage of non-existent block!");
    }
}
} // namespace tests
