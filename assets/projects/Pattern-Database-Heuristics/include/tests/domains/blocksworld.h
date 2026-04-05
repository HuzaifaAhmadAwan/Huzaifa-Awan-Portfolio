#ifndef DOMAINS_BLOCKSWORLD_H
#define DOMAINS_BLOCKSWORLD_H

#include "tests/domains/classical_planning_domain.h"

#include "downward/task_proxy.h"

#include <vector>

namespace tests {

/**
 * @brief Models the planning domain BlocksWorld.
 *
 * A BlocksWorld problem consists of \f$N\f$ blocks arranged in stacks.
 * The goal is to rearrange the blocks to adhere to a specific configuration of
 * stacks, usually a single stack with blocks appearing in a specific order.
 *
 * There are two kinds of actions:
 *
 * 1. We can *pick* a block on top of a stack so long as we are not already
 *    holding a block.
 * 2. We can *put* a held block onto a stack or the table.
 *
 * This environment is modelled as a classical FDR planning task as follows:
 *
 * Variables:
 * + \f$\textsf{HandEmpty} \in \{ \text{true}, \text{false} \}\f$ - Whether
 * a block is currently held.
 * + \f$\textsf{Location}(b) \in \{ 0, \dots, N-1 \} \cup
 * \{ \text{table}, \text{hand} \} \f$ - The location of the block
 * \f$b \in \{ 0, \dots, N-1 \}\f$. Either
 * stacked on another block \f$b' \in \{ 0, \dots, N-1 \}\f$, on the table or in
 * the hand.
 * + \f$\textsf{Clear}(b) \in \{ \text{true}, \text{false} \}\f$ - Whether there
 * is another block \f$b'\f$ on top of \f$b \in \{ 0, \dots, N-1 \}\f$. Either
 * true or false.
 *
 * Operators:
 * + \f$\textsf{Pick-block}(b, b')\f$ - Picks the block \f$b \in \{ 0, \dots,
 * N-1 \}\f$ that is currently stacked on \f$b' \in \{ 0, \dots, N-1 \}\f$.
 *   - Precondition: \f$\textsf{HandEmpty}=true, \textsf{Location}(b)=b',
 * \textsf{Clear}(b)=\text{true}\f$
 *   - Effect:
 *    \f$\textsf{HandEmpty}=\text{false},
 *    \textsf{Location}(b)=\text{hand},
 *    \textsf{Clear}(b)=\text{false},
 *    \textsf{Clear}(b')=\text{true}\f$
 * + \f$\textsf{Pick-table}(b)\f$ - Picks the block \f$b \in \{ 0, \dots, N-1
 * \}\f$ that is currently on the table.
 *   - Precondition:
 *     \f$\textsf{HandEmpty}=\text{true},
 *     \textsf{Location}(b)=\text{table},
 *     \textsf{Clear}(b)=\text{true}\f$
 *   - Effect:
 *     \f$\textsf{HandEmpty}=\text{false},
 *     \textsf{Location}(b)=\text{hand},
 *     \textsf{Clear}(b)=\text{false}\f$
 * + \f$\textsf{Put-block}(b, b')\f$ - Puts the block \f$b \in \{ 0, \dots, N-1
 * \}\f$ onto \f$b' \in \{ 0, \dots, N-1 \}\f$.
 *   - Precondition:
 *     \f$\textsf{Location}(b)=\text{hand},
 *     Clear(b')=\text{true}\f$
 *   - Effect:
 *     \f$\textsf{HandEmpty}=true,
 *     \textsf{Location}(b)=b',
 *     \textsf{Clear}(b)=\text{true},
 *     \textsf{Clear}(b')=\text{false}\f$
 * + \f$\textsf{Put-table}(b)\f$ - Puts the block \f$b \in \{ 0, \dots, N-1
 * \}\f$ on the table.
 *   - Precondition: \f$\textsf{Location}(b)=\text{hand}\f$
 *   - Effect:
 *     \f$\textsf{HandEmpty}=\text{true},
 *     \textsf{Location}(b)=\text{table},
 *     \textsf{Clear}(b)=\text{true}\f$
 *
 * All operators have unit costs by default, but can be configured to have
 * different costs as well. See constructor arguments for details.
 *
 * @ingroup test_domains
 */
class BlocksWorld : public ClassicalPlanningDomain {
    int num_blocks;

public:
    /// Constructs a BlocksWorld problem with the specified number of blocks
    /// \f$N\f$ and action costs.
    explicit BlocksWorld(int num_blocks, int pick_cost = 1, int put_cost = 1);

    /// Get the variable index for \f$\textsf{HandEmpty}\f$.
    int get_variable_hand_empty() const;

    /// Get the variable index for \f$\textsf{Location}(b)\f$.
    int get_variable_location(int block_number) const;

    /// Get the variable index for \f$\textsf{Clear}(b)\f$.
    int get_variable_clear(int block_number) const;

    /// Get the fact \f$\textsf{Clear}(b) = B\f$, where
    /// \f$B \in \{ \text{true}, \text{false} \}\f$.
    FactPair get_fact_is_clear(int block_number, bool is_clear) const;

    /// Get the fact \f$\textsf{Location}(b) = b'\f$,
    /// where \f$b' \in \{ 0, \dots, N - 1 \}\f$ is another block.
    FactPair
    get_fact_location_on_block(int block_number, int block_below) const;

    /// Get the fact \f$\textsf{Location}(b) = \text{table}\f$.
    FactPair get_fact_location_on_table(int block_number) const;

    /// Get the fact \f$\textsf{Location}(b) = \text{hand}\f$.
    FactPair get_fact_location_in_hand(int block_number) const;

    /// Get the fact \f$\textsf{HandEmpty}(b) = B\f$,
    /// where \f$B \in \{ \text{true}, \text{false} \}\f$.
    FactPair get_fact_is_hand_empty(bool is_empty) const;

    /// Get the operator index for the operator
    /// \f$\textsf{Pick-block}(b, b')\f$.
    OperatorID get_operator_id_pick_from_block(int b1, int b2) const;

    /// Get the operator index for the operator
    /// \f$\textsf{Pick-table}(b)\f$.
    OperatorID get_operator_id_pick_from_table(int b) const;

    /// Get the operator index for the operator
    /// \f$\textsf{Put-block}(b, b')\f$.
    OperatorID get_operator_id_put_on_block(int b1, int b2) const;

    /// Get the operator index for the operator
    /// \f$\textsf{Put-table}(b)\f$.
    OperatorID get_operator_id_put_on_table(int b) const;

private:
    int get_operator_index_pick_from_block(int b1, int b2) const;
    int get_operator_index_pick_from_table(int b) const;
    int get_operator_index_put_on_block(int b1, int b2) const;
    int get_operator_index_put_on_table(int b) const;

    void verify_block_index(int block) const;
};

} // namespace tests

#endif // DOMAINS_BLOCKSWORLD_H
