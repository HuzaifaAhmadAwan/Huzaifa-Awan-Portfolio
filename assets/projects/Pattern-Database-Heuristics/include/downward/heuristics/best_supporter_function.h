
#ifndef DOWNWARD_BEST_SUPPORTER_FUNCTION_H
#define DOWNWARD_BEST_SUPPORTER_FUNCTION_H

#include "downward/operator_id.h"

#include <optional>

struct FactPair;
class State;

/**
 * @brief Represents a best supporter function.
 *
 * In FDR, a best supporter function for a state \f$s\f$ is a partial function
 * \f[
 * bs : \facts \setminus s \rightharpoonup \ops
 * \f]
 * such that \f$g \in \eff_a\f$ whenever \f$a = bs(g)\f$.
 * Here \f$\facts\f$ is the set of facts of the planning task and
 * \f$\facts \setminus s\f$ are all facts not true in \f$s\f$.
 *
 * @ingroup heuristics
 */
class BestSupporterFunction {
public:
    virtual ~BestSupporterFunction() = default;

    /**
     * @brief Re-computes the best supporter function for the given initial
     * state \f$s\f$.
     *
     * After calling this function, calling \ref get_best_supporter will yield
     * the best supporter of the specified fact with respect to the inital state
     * given by this function.
     */
    virtual void compute_best_supporters(const State& state) = 0;

    /**
     * @brief Returns the best supporter of a fact, with respect to the last
     * initial state \ref compute_best_supporters was called on.
     *
     * Returns `std::nullopt` if the best supporter does not exist, otherwise
     * some best supporter as an OperatorID.
     *
     * @warning The behaviour of this function is unspecified if no prior call
     * to `compute_best_supporters` has been made.
     */
    virtual std::optional<OperatorID>
    get_best_supporter(const FactPair& fact) = 0;
};

#endif
