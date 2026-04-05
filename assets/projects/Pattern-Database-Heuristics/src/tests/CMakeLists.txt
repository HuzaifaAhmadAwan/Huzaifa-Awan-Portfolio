cmake_minimum_required(VERSION 3.22)

#######################################################################################
# Obtain GoogleTest                                                                   #
#######################################################################################
include(FetchContent)

# Show fetch output
Set(FETCHCONTENT_QUIET FALSE)

# Specify repo to clone form and commit
FetchContent_Declare(
    GTest
    GIT_REPOSITORY https://github.com/google/googletest.git  # official GoogleTest repository
    GIT_TAG        b514bdc898e2951020cbdca1304b75f5950d1f59  # release-1.15.0 (HEAD of branch v1.15.x)
    GIT_PROGRESS   TRUE                                      # show git clone progress
    UPDATE_DISCONNECTED TRUE                                 # do not pull on re-configuration
    #FIND_PACKAGE_ARGS                                        # use find_package(GTest) first
)

# This creates the target gtest which is the target of the GoogleTest library
FetchContent_MakeAvailable(GTest)
#######################################################################################

# Executable target for the test executable
add_executable(project_tests ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.cc)
target_link_libraries(project_tests PRIVATE common_cxx_flags GTest::gtest)

# On Windows we have to copy all DLLs next to the generated binary.
if (WIN32)
    copy_dlls_to_binary_dir_after_build(project_tests)
endif()

# add basic test sources (cmake directory)
include(TestBaseFiles)

# add sources of the student tests, if existent (cmake directory)
include(StudentTests OPTIONAL)

# add sources of the existing projects (cmake directory)
include(TestFilesPZero OPTIONAL)
include(TestFilesPOne OPTIONAL)
include(TestFilesPTwo OPTIONAL)
include(TestFilesPThree OPTIONAL)
include(TestFilesPFour OPTIONAL)

# Register all tests with ctest
include(GoogleTest)

gtest_discover_tests(
    project_tests
    DISCOVERY_TIMEOUT 45
    DISCOVERY_MODE PRE_TEST
    NO_PRETTY_VALUES
)