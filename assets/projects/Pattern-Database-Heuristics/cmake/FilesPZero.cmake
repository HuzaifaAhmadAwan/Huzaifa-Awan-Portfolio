create_library(
    NAME blind_search_heuristic
    HELP "The 'blind search' heuristic"
    SOURCES
        downward/heuristics/blind_search_heuristic
    DEPENDS
        task_properties
    TARGET
        downward
)

create_library(
    NAME goal_count_heuristic
    HELP "The goal-counting heuristic"
    SOURCES
        downward/heuristics/goal_count_heuristic
    TARGET
        downward
)
