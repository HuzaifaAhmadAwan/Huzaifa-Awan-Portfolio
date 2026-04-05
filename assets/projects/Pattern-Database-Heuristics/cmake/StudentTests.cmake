create_library(
    NAME student_tests
    HELP "Local student tests"
    SOURCES
        tests/student/example_tests
    DEPENDS
        GTest::gtest
        test_domains           # Basic dependencies
        task_utils             # Basic dependencies
        input_utils            # Basic dependencies
        heuristic_test_utils   # Basic dependencies
        search_test_utils      # Basic dependencies
        # blind_search_heuristic # Project 0 dependencies
        # goal_count_heuristic   # Project 0 dependencies
        # max_heuristic          # Project 1 dependencies
        # ff_heuristic           # Project 1 dependencies
        # pdbs                   # Project 2 dependencies
        # q_learning             # Project 4 dependencies
        # torch_networks         # Project 4 dependencies

        # ff_test_utils          # Project 1 dependencies
        # pdb_test_utils         # Project 2 dependencies
        # q_learning_test_utils  # Project 4 dependencies
    TARGET project_tests
)