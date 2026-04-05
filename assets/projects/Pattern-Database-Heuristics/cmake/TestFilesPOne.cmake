create_library(
    NAME ff_test_utils
    HELP "Utility for the FF heuristic tests"
    SOURCES
        tests/utils/ff_utils
    DEPENDS
        ff_heuristic
    TARGET project_tests
)

create_library(
    NAME hmax_public_tests
    HELP "hmax public tests"
    SOURCES
        tests/public/heuristic_tests/hmax_tests
    DEPENDS
        GTest::gtest
        max_heuristic
        test_domains
        task_utils
        search_test_utils
        heuristic_test_utils
    TARGET project_tests
)

create_library(
    NAME hff_public_tests
    HELP "hFF public tests"
    SOURCES
        tests/public/heuristic_tests/hff_tests
    DEPENDS
        GTest::gtest
        ff_heuristic
        test_domains
        task_utils
        search_test_utils
        heuristic_test_utils
        ff_test_utils
    TARGET project_tests
)
