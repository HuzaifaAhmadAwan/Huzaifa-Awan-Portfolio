#ifndef SEARCH_SPACE_H
#define SEARCH_SPACE_H

#include "downward/operator_cost.h"
#include "downward/per_state_information.h"
#include "downward/search_node_info.h"

#include <vector>

class OperatorProxy;
class State;
class ClassicalPlanningTask;

namespace utils {
class LogProxy;
}

class SearchNode {
    State state;
    SearchNodeInfo& info;

public:
    SearchNode(const State& state, SearchNodeInfo& info);

    const State& get_state() const;

    bool is_new() const;
    bool is_open() const;
    bool is_closed() const;
    bool is_dead_end() const;

    int get_g() const;
    int get_real_g() const;

    void open_initial();
    void open(
        const SearchNode& parent_node,
        const OperatorProxy& parent_op,
        int adjusted_cost);
    void reopen(
        const SearchNode& parent_node,
        const OperatorProxy& parent_op,
        int adjusted_cost);
    void update_parent(
        const SearchNode& parent_node,
        const OperatorProxy& parent_op,
        int adjusted_cost);
    void close();
    void mark_as_dead_end();

    void dump(const ClassicalPlanningTask& task, utils::LogProxy& log) const;

    // Needed by NeuralFD
    void reopen();
};

class SearchSpace {
    PerStateInformation<SearchNodeInfo> search_node_infos;

    StateRegistry& state_registry;
    utils::LogProxy& log;

public:
    SearchSpace(StateRegistry& state_registry, utils::LogProxy& log);

    SearchNode get_node(const State& state);

    void
    trace_path(const State& goal_state, std::vector<OperatorID>& path) const;

    void dump(const ClassicalPlanningTask& task) const;
    void print_statistics() const;

    // NeuralFD hacks

    StateID get_parent_id(const State& state) const
    {
        return search_node_infos[state].parent_state_id;
    }

    OperatorID get_creating_operator(const State& state) const
    {
        return search_node_infos[state].creating_operator;
    }

    void
    trace_path(const State& goal_state, std::vector<StateID>& trajectory) const;

    void trace_path(
        const State& goal_state,
        std::vector<OperatorID>& path,
        std::vector<StateID>& trajectory) const;
};

#endif
