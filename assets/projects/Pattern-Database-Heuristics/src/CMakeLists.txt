# See https://www.fast-downward.org/ForDevelopers/AddingSourceFiles
# for general information on adding source files and CMake libraries.
#
# Defining a new library:
#    < Test library >:
#    create_fast_downward_library(
#        NAME <NAME>
#        [ HELP <HELP> ]
#        SOURCES
#            <FILE_1> [ <FILE_2> ... ]
#        [ DEPENDS <LIBRARY_NAME_1> [ <LIBRARY_NAME_2> ... ] ]
#        [ DEPENDENCY_ONLY ]
#        [ CORE_LIBRARY ]
#    )
#
#    < Test library >:
#    create_fast_downward_library(
#        NAME <NAME>
#        [ HELP <HELP> ]
#        SOURCES
#            <FILE_1> [ <FILE_2> ... ]
#        [ DEPENDS <LIBRARY_NAME_1> [ <LIBRARY_NAME_2> ... ] ]
#        [ DEPENDENCY_ONLY ]
#        [ CORE_LIBRARY ]
#    )
#
#    < Test library >:
#    create_fast_downward_library(
#        NAME <NAME>
#        [ HELP <HELP> ]
#        SOURCES
#            <FILE_1> [ <FILE_2> ... ]
#        [ DEPENDS <LIBRARY_NAME_1> [ <LIBRARY_NAME_2> ... ] ]
#        [ DEPENDENCY_ONLY ]
#        [ CORE_LIBRARY ]
#    )
#
# SOURCES lists the source files that are part of the library. Entries are
#    listed without extension. For an entry <file>, both <file>.h and <file>.cc
#    are added if the files exist.
# DEPENDS lists libraries that will be compiled as dependendies if this library
#    is enabled.
# DEPENDENCY_ONLY disables the library unless it is needed as a dependency and
#    hides the option to enable the library in cmake GUIs like ccmake.
# CORE_LIBRARY always enables the library (even if DISABLE_LIBRARIES_BY_DEFAULT
#    is used) and hides the option to disable it in CMake GUIs like ccmake.

cmake_minimum_required(VERSION 3.22)

# Set include directory to include/
target_include_directories(common_cxx_flags INTERFACE ${PROJECT_SOURCE_DIR}/include/)

# Executable target for the fast-downward planner
add_executable(downward ${CMAKE_CURRENT_SOURCE_DIR}/downward/planner.cc)

# On Windows we have to copy all DLLs next to the generated binary.
if (WIN32)
    copy_dlls_to_binary_dir_after_build(downward)
endif()

# Collect source files of all components.
include(DownwardFiles)
include(ProbFDFiles)

include(FilesPZero OPTIONAL)
include(FilesPOne OPTIONAL)
include(FilesPTwo OPTIONAL)
include(FilesPThree OPTIONAL)
include(FilesPFour OPTIONAL)

# Add testing component seperately.
add_subdirectory(tests)