
#ifndef DOWNWARD_PDBS_STATE_ENUMERATOR_H
#define DOWNWARD_PDBS_STATE_ENUMERATOR_H

#include <memory>
#include <vector>

class ClassicalPlanningTask;
class State;

namespace pdbs {

/**
 * @brief This class can be used to enumerate the states of a planning task's
 * transition system.
 */
class StateEnumerator {
    std::shared_ptr<ClassicalPlanningTask> task_;
    std::vector<std::size_t> multipliers_;
    std::size_t num_states_;

public:
    /// Constructs a StateEnumerator for a given planning task.
    explicit StateEnumerator(const std::shared_ptr<ClassicalPlanningTask>& task);

    /// Computes an unique index for the input abstract state in the range
    /// \f$\{0, \dots, |S| - 1\}\f$, where \f$S\f$ is are the states of the
    /// planning task's transition system, i.e., the set of complete variable
    /// assignments.with respect to the task's variables.
    /// \attention This method may only be called with a state of the planning
    /// task object the enumerator was constructed with.
    /// Calling this method with a state associated with a different task leads
    /// to undefined behaviour.
    ///
    /// Time complexity: \f$ \mathcal{O}(|V|) \f$
    /// \return
    std::size_t get_index(const State& state) const;

    /// Computes the unique abstract state with the given index
    /// \f$\{0, \dots, |S| - 1\}\f$, where \f$S\f$ is are the states of the
    /// planning task's transition system, i.e., the set of complete variable
    /// assignments.with respect to the task's variables.
    State get_state(std::size_t index) const;

    /// Get the number of states \f$|S|\f$ of the planning task's transition
    /// system, i.e., the set of complete variable assignments.with respect to
    /// the task's variables.
    std::size_t num_states() const;
};

} // namespace pdbs

#endif
