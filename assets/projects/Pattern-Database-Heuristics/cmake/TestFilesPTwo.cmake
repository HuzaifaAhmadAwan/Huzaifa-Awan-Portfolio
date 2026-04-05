create_library(
    NAME pdb_test_utils
    HELP "Utility for the PDB heuristic tests"
    SOURCES
        tests/utils/pdb_utils
    DEPENDS
        GTest::gtest
        pdbs
    TARGET project_tests
)

create_library(
    NAME pdb_heuristic_public_tests
    HELP "PDB heuristic public tests"
    SOURCES
        tests/public/heuristic_tests/pdb_tests
    DEPENDS
        GTest::gtest
        pdb_heuristic
        test_domains
        task_utils
        search_test_utils
        heuristic_test_utils
    TARGET project_tests
)

create_library(
    NAME hstar_table_public_tests
    HELP "H-Star Table public tests"
    SOURCES
        tests/public/heuristic_tests/hstar_table_tests
    DEPENDS
        GTest::gtest
        pdbs
        test_domains
        task_utils
    TARGET project_tests
)

create_library(
    NAME cliques_heuristic_public_tests
    HELP "Cliques heuristic public tests"
    SOURCES
        tests/public/heuristic_tests/cliques_heuristic_tests
    DEPENDS
        GTest::gtest
        cliques_heuristic
        test_domains
        task_utils
        search_test_utils
        heuristic_test_utils
    TARGET project_tests
)
