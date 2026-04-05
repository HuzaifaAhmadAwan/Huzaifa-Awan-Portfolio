#include "downward/state_registry.h"

#include "downward/per_state_information.h"
#include "downward/task_proxy.h"

#include "downward/utils/logging.h"

using namespace std;

StateRegistry::StateRegistry(const AbstractPlanningTask& task)
    : task(task)
    , state_packer(task_properties::g_state_packers[task])
    , num_variables(task.get_variables().size())
    , state_data_pool(get_bins_per_state())
    , registered_states(
          StateIDSemanticHash(state_data_pool, get_bins_per_state()),
          StateIDSemanticEqual(state_data_pool, get_bins_per_state()))
{
}

StateID StateRegistry::insert_id_or_pop_state()
{
    /*
      Attempt to insert a StateID for the last state of state_data_pool
      if none is present yet. If this fails (another entry for this state
      is present), we have to remove the duplicate entry from the
      state data pool.
    */
    StateID id(state_data_pool.size() - 1);
    pair<int, bool> result = registered_states.insert(id.value);
    bool is_new_entry = result.second;
    if (!is_new_entry) {
        state_data_pool.pop_back();
    }
    assert(
        registered_states.size() == static_cast<int>(state_data_pool.size()));
    return StateID(result.first);
}

State StateRegistry::lookup_state(StateID id) const
{
    const PackedStateBin* buffer = state_data_pool[id.value];
    return task.create_state(*this, id, buffer);
}

const State& StateRegistry::get_initial_state()
{
    if (!cached_initial_state) {
        int num_bins = get_bins_per_state();
        unique_ptr<PackedStateBin[]> buffer(new PackedStateBin[num_bins]);
        // Avoid garbage values in half-full bins.
        fill_n(buffer.get(), num_bins, 0);

        State initial_state = task.get_initial_state();
        for (size_t i = 0; i < initial_state.size(); ++i) {
            state_packer.set(buffer.get(), i, initial_state[i]);
        }
        state_data_pool.push_back(buffer.get());
        StateID id = insert_id_or_pop_state();
        cached_initial_state = std::make_unique<State>(lookup_state(id));
    }
    return *cached_initial_state;
}

State StateRegistry::insert_state(std::vector<int>&& state)
{
    int num_bins = get_bins_per_state();
    unique_ptr<PackedStateBin[]> buffer(new PackedStateBin[num_bins]);
    // Avoid garbage values in half-full bins.
    fill_n(buffer.get(), num_bins, 0);

    for (size_t i = 0; i < state.size(); ++i) {
        state_packer.set(buffer.get(), i, state[i]);
    }
    state_data_pool.push_back(buffer.get());
    // buffer is copied by push_back
    StateID id = insert_id_or_pop_state();
    return lookup_state(id);
}

// TODO it would be nice to move the actual state creation (and operator
// application)
//      out of the StateRegistry. This could for example be done by global
//      functions operating on state buffers (PackedStateBin *).
State StateRegistry::get_successor_state(
    const State& predecessor,
    const OperatorProxy& op)
{
    state_data_pool.push_back(predecessor.get_buffer());
    /*
      TODO: ideally, we would not modify state_data_pool here and in
      insert_id_or_pop_state, but only at one place, to avoid errors like
      buffer becoming a dangling pointer. This used to be a bug before being
      fixed in https://issues.fast-downward.org/issue1115.
    */
    PackedStateBin* buffer = state_data_pool[state_data_pool.size() - 1];
    /* Experiments for issue348 showed that for domains with axioms it's faster
       to compute successor states using unpacked data. */

    for (FactProxy effect : op.get_effects()) {
        FactPair effect_pair = effect.get_pair();
        state_packer.set(buffer, effect_pair.var, effect_pair.value);
    }

    /*
      NOTE: insert_id_or_pop_state possibly invalidates buffer, hence
      we use lookup_state to retrieve the state using the correct buffer.
    */
    StateID id = insert_id_or_pop_state();
    return lookup_state(id);
}

int StateRegistry::get_bins_per_state() const
{
    return state_packer.get_num_bins();
}

int StateRegistry::get_state_size_in_bytes() const
{
    return get_bins_per_state() * sizeof(PackedStateBin);
}

void StateRegistry::print_statistics(utils::LogProxy& log) const
{
    log << "Number of registered states: " << size() << endl;
    registered_states.print_statistics(log);
}
