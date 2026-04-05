create_library(
    NAME best_supporter_function
    HELP "Best supporter functions"
    SOURCES
        downward/heuristics/best_supporter_function
    TARGET
        downward
)

create_library(
    NAME delete_relaxed_plan_heuristic
    HELP "Delete-relaxed plan heuristics"
    SOURCES
        downward/heuristics/delete_relaxed_plan_heuristic
)

create_library(
    NAME ff_heuristic
    HELP "The FF heuristic (an implementation of the RPG heuristic)"
    SOURCES
        downward/heuristics/ff_heuristic
    DEPENDS
        delete_relaxed_plan_heuristic
        task_properties
        best_supporter_function
    TARGET
        downward
)

create_library(
    NAME max_heuristic
    HELP "The Max heuristic"
    SOURCES
        downward/heuristics/max_heuristic
    DEPENDS
        priority_queues
        best_supporter_function
    TARGET
        downward
)
