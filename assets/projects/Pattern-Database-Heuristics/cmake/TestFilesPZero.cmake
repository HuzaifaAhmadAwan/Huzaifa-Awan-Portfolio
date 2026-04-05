create_library(
    NAME blind_heuristic_public_tests
    HELP "Blind heuristic public tests"
    SOURCES
        tests/public/heuristic_tests/blind_heuristic_tests
    DEPENDS
        GTest::gtest
        blind_search_heuristic
        test_domains
        task_utils
        heuristic_test_utils
    TARGET project_tests
)

create_library(
    NAME goal_counting_heuristic_public_tests
    HELP "Goal-counting heuristic public tests"
    SOURCES
        tests/public/heuristic_tests/goal_count_tests
    DEPENDS
        GTest::gtest
        goal_count_heuristic
        test_domains
        task_utils
        search_test_utils
        heuristic_test_utils
    TARGET project_tests
)
