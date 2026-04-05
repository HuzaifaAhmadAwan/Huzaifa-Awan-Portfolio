create_library(
    NAME test_domains
    HELP "Test planning domains"
    SOURCES
        tests/domains/classical_planning_domain
        tests/domains/blocksworld
        tests/domains/gripper
        tests/domains/nomystery
        tests/domains/visitall
        tests/domains/sokoban
    DEPENDS
        core_sources
        plugins
        parser
        utils
        core_tasks
    TARGET project_tests
    CORE_LIBRARY
)

create_library(
    NAME task_utils
    HELP "Utility for test tasks"
    SOURCES
        tests/utils/task_utils
    TARGET project_tests
)

create_library(
    NAME input_utils
    HELP "Utility for input processing"
    SOURCES
        tests/utils/input_utils
    TARGET project_tests
)

create_library(
    NAME heuristic_test_utils
    HELP "Utility for heuristic tests"
    SOURCES
        tests/utils/heuristic_utils
    DEPENDS
        core_sources
    TARGET project_tests
)

create_library(
    NAME search_test_utils
    HELP "Utility for search tests"
    SOURCES
        tests/utils/search_utils
    DEPENDS
        search_common
        eager_search
    TARGET project_tests
)
