#include "downward/pdbs/hstar.h"

#include "downward/pdbs/state_enumerator.h"

#include "downward/heuristic.h"
#include "downward/state.h"
#include "downward/task_proxy.h"

#include <queue>
#include <vector>
#include <limits>

namespace pdbs {

std::vector<int> compute_hstar_table(
    const ClassicalPlanningTask& task,
    const StateEnumerator& enumerator)
{
    const std::size_t num_states = enumerator.num_states();
    std::vector<int> h_star(num_states, Heuristic::DEAD_END);

    // Build reverse graph: for each state, store which states can lead to it
    // The edges are weighted by operator cost
    std::vector<std::vector<std::pair<std::size_t, int>>> reverse_graph(num_states);
    
    for (std::size_t pred_idx = 0; pred_idx < num_states; ++pred_idx) {
        State pred_state = enumerator.get_state(pred_idx);
        
        // For each operator, check if it's applicable
        for (OperatorProxy op : task.get_operators()) {
            bool applicable = true;
            for (FactProxy precond : op.get_preconditions()) {
                if (pred_state[precond.get_variable().get_id()] 
                    != precond.get_value()) {
                    applicable = false;
                    break;
                }
            }
            
            if (applicable) {
                // Apply operator to get successor
                State succ_state = get_unregistered_successor(pred_state, op);
                std::size_t succ_idx = enumerator.get_index(succ_state);
                
                // Add edge in reverse graph: from successor to predecessor
                reverse_graph[succ_idx].push_back({pred_idx, op.get_cost()});
            }
        }
    }

    // Initialize priority queue for Dijkstra's algorithm
    // Pair: (distance, state_index)
    auto cmp = [](const std::pair<int, std::size_t>& a,
                  const std::pair<int, std::size_t>& b) {
        return a.first > b.first;  // Min-heap
    };
    std::priority_queue<std::pair<int, std::size_t>,
                        std::vector<std::pair<int, std::size_t>>,
                        decltype(cmp)> pq(cmp);

    // Find goal states and initialize with distance 0
    for (std::size_t i = 0; i < num_states; ++i) {
        State state = enumerator.get_state(i);
        
        // Check if state is a goal state
        bool is_goal = true;
        for (FactProxy goal : task.get_goal()) {
            if (state[goal.get_variable().get_id()] != goal.get_value()) {
                is_goal = false;
                break;
            }
        }
        
        if (is_goal) {
            h_star[i] = 0;
            pq.push({0, i});
        }
    }

    // Dijkstra's algorithm on reverse graph
    std::vector<bool> visited(num_states, false);

    while (!pq.empty()) {
        auto [dist, state_idx] = pq.top();
        pq.pop();

        if (visited[state_idx]) {
            continue;
        }
        visited[state_idx] = true;

        // Explore predecessors in the reverse graph
        for (const auto& [pred_idx, cost] : reverse_graph[state_idx]) {
            if (!visited[pred_idx]) {
                int new_cost = dist + cost;
                if (h_star[pred_idx] == Heuristic::DEAD_END 
                    || new_cost < h_star[pred_idx]) {
                    h_star[pred_idx] = new_cost;
                    pq.push({new_cost, pred_idx});
                }
            }
        }
    }

    return h_star;
}

} // namespace pdbs
