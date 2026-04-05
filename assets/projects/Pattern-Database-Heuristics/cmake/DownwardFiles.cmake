create_library(
    NAME core_sources
    HELP "Core source files"
    SOURCES
        downward/abstract_task
        downward/cached_heuristic
        downward/command_line
        downward/evaluation_context
        downward/evaluation_result
        downward/evaluator
        downward/evaluator_base
        downward/evaluator_cache
        downward/fact_pair
        downward/fact_enumerator
        downward/heuristic
        downward/open_list
        downward/open_list_factory
        downward/operator_cost
        downward/operator_id
        downward/per_state_array
        downward/per_state_bitset
        downward/per_state_information
        downward/per_task_information
        downward/plan_manager
        downward/search_algorithm
        downward/search_node_info
        downward/search_progress
        downward/search_space
        downward/search_statistics
        downward/state
        downward/state_id
        downward/state_registry
        downward/task_id
        downward/task_proxy
    DEPENDS causal_graph int_hash_set int_packer ordered_set segmented_vector subscriber successor_generator task_properties
    TARGET downward
    CORE_LIBRARY
)

create_library(
    NAME plugins
    HELP "Plugin definition"
    SOURCES
        downward/plugins/any
        downward/plugins/bounds
        downward/plugins/doc_printer
        downward/plugins/options
        downward/plugins/plugin
        downward/plugins/plugin_info
        downward/plugins/raw_registry
        downward/plugins/registry
        downward/plugins/registry_types
        downward/plugins/types
    TARGET downward
    CORE_LIBRARY
)

create_library(
    NAME parser
    HELP "Option parsing"
    SOURCES
        downward/parser/abstract_syntax_tree
        downward/parser/decorated_abstract_syntax_tree
        downward/parser/lexical_analyzer
        downward/parser/syntax_analyzer
        downward/parser/token_stream
    TARGET downward
    CORE_LIBRARY
)

create_library(
    NAME utils
    HELP "System utilities"
    SOURCES
        downward/utils/collections
        downward/utils/countdown_timer
        downward/utils/exceptions
        downward/utils/hash
        downward/utils/logging
        downward/utils/markup
        downward/utils/math
        downward/utils/memory
        downward/utils/not_implemented
        downward/utils/rng
        downward/utils/rng_options
        downward/utils/strings
        downward/utils/system
        downward/utils/system_unix
        downward/utils/system_windows
        downward/utils/timer
        downward/utils/tuples
    TARGET downward
    CORE_LIBRARY
)

# On Linux, find the rt library for clock_gettime().
if(UNIX AND NOT APPLE)
    target_link_libraries(utils INTERFACE rt)
endif()

# On Windows, find the psapi library for determining peak memory.
if(WIN32)
    cmake_policy(SET CMP0074 NEW)
    target_link_libraries(utils INTERFACE psapi)
endif()

create_library(
    NAME alternation_open_list
    HELP "Open list that alternates between underlying open lists in a round-robin manner"
    SOURCES
        downward/open_lists/alternation_open_list
    TARGET downward
)

create_library(
    NAME best_first_open_list
    HELP "Open list that selects the best element according to a single evaluation function"
    SOURCES
        downward/open_lists/best_first_open_list
    TARGET downward
)

create_library(
    NAME epsilon_greedy_open_list
    HELP "Open list that chooses an entry randomly with probability epsilon"
    SOURCES
        downward/open_lists/epsilon_greedy_open_list
    TARGET downward
)

create_library(
    NAME pareto_open_list
    HELP "Pareto open list"
    SOURCES
        downward/open_lists/pareto_open_list
    TARGET downward
)

create_library(
    NAME tiebreaking_open_list
    HELP "Tiebreaking open list"
    SOURCES
        downward/open_lists/tiebreaking_open_list
    TARGET downward
)

create_library(
    NAME type_based_open_list
    HELP "Type-based open list"
    SOURCES
        downward/open_lists/type_based_open_list
    TARGET downward
)

create_library(
    NAME int_hash_set
    HELP "Hash set storing non-negative integers"
    SOURCES
        downward/algorithms/int_hash_set
)

create_library(
    NAME int_packer
    HELP "Greedy bin packing algorithm to pack integer variables with small domains tightly into memory"
    SOURCES
        downward/algorithms/int_packer
)

create_library(
    NAME max_cliques
    HELP "Implementation of the Max Cliques algorithm by Tomita et al."
    SOURCES
        downward/algorithms/max_cliques
)

create_library(
    NAME priority_queues
    HELP "Three implementations of priority queue: HeapQueue, BucketQueue and AdaptiveQueue"
    SOURCES
        downward/algorithms/priority_queues
)

create_library(
    NAME ordered_set
    HELP "Set of elements ordered by insertion time"
    SOURCES
        downward/algorithms/ordered_set
)

create_library(
    NAME segmented_vector
    HELP "Memory-friendly and vector-like data structure"
    SOURCES
        downward/algorithms/segmented_vector
)

create_library(
    NAME subscriber
    HELP "Allows object to subscribe to the destructor of other objects"
    SOURCES
        downward/algorithms/subscriber
)

create_library(
    NAME evaluators_subcategory
    HELP "Subcategory plugin for basic evaluators"
    SOURCES
        downward/evaluators/subcategory
    TARGET downward
)

create_library(
    NAME const_evaluator
    HELP "The constant evaluator"
    SOURCES
        downward/evaluators/const_evaluator
    DEPENDS evaluators_subcategory
    TARGET downward
)

create_library(
    NAME g_evaluator
    HELP "The g-evaluator"
    SOURCES
        downward/evaluators/g_evaluator
    DEPENDS evaluators_subcategory
    TARGET downward
)

create_library(
    NAME combining_evaluator
    HELP "The combining evaluator"
    SOURCES
        downward/evaluators/combining_evaluator
)

create_library(
    NAME max_evaluator
    HELP "The max evaluator"
    SOURCES
        downward/evaluators/max_evaluator
    DEPENDS combining_evaluator evaluators_subcategory
    TARGET downward
)

create_library(
    NAME weighted_evaluator
    HELP "The weighted evaluator"
    SOURCES
        downward/evaluators/weighted_evaluator
    DEPENDS evaluators_subcategory
    TARGET downward
)

create_library(
    NAME sum_evaluator
    HELP "The sum evaluator"
    SOURCES
        downward/evaluators/sum_evaluator
    DEPENDS combining_evaluator evaluators_subcategory
    TARGET downward
)

create_library(
    NAME search_common
    HELP "Basic classes used for all search engines"
    SOURCES
        downward/search_algorithms/search_common
    DEPENDS alternation_open_list g_evaluator best_first_open_list sum_evaluator tiebreaking_open_list weighted_evaluator
)

create_library(
    NAME eager_search
    HELP "Eager search algorithm"
    SOURCES
        downward/search_algorithms/eager_search
    DEPENDS ordered_set successor_generator
)

create_library(
    NAME plugin_astar
    HELP "A* search"
    SOURCES
        downward/search_algorithms/plugin_astar
    DEPENDS eager_search search_common
    TARGET downward
)

create_library(
    NAME plugin_eager
    HELP "Eager (i.e., normal) best-first search"
    SOURCES
        downward/search_algorithms/plugin_eager
    DEPENDS eager_search search_common
    TARGET downward
)

create_library(
    NAME plugin_eager_greedy
    HELP "Eager greedy best-first search"
    SOURCES
        downward/search_algorithms/plugin_eager_greedy
    DEPENDS eager_search search_common
    TARGET downward
)

create_library(
    NAME plugin_eager_wastar
    HELP "Weighted eager A* search"
    SOURCES
        downward/search_algorithms/plugin_eager_wastar
    DEPENDS eager_search search_common
    TARGET downward
)

create_library(
    NAME core_tasks
    HELP "Core task transformations"
    SOURCES
        downward/tasks/cost_adapted_task
        downward/tasks/delegating_task
        downward/tasks/root_task
    CORE_LIBRARY
    TARGET downward
)

create_library(
    NAME causal_graph
    HELP "Causal Graph"
    SOURCES
        downward/task_utils/causal_graph
)

create_library(
    NAME sampling
    HELP "Sampling"
    SOURCES
        downward/task_utils/sampling
    DEPENDS successor_generator task_properties
)

create_library(
    NAME successor_generator
    HELP "Successor generator"
    SOURCES
        downward/task_utils/successor_generator
        downward/task_utils/successor_generator_factory
        downward/task_utils/successor_generator_internals
    DEPENDS task_properties
)

create_library(
    NAME task_properties
    HELP "Task properties"
    SOURCES
        downward/task_utils/task_properties
)

create_library(
    NAME variable_order_finder
    HELP "Variable order finder"
    SOURCES
        downward/task_utils/variable_order_finder
)