# Usage:
#         cmake -S src -B builds/release
#         cmake --build builds/release
# The first call caches settings in the build directory and reads
# them from the cache on subsequent builds. If you want to change the
# settings of some options, do _not_ change the CMakeLists.txt files.
# Instead, create a new build directory, pass -DMY_OPTION=my_value to
# cmake. Alternatively, you can use a cmake GUI like ccmake to edit
# the cache.
#
# Two build targets are defined:
#
# * release (default)
#      -O3 optimisation, debugging symbols, assertions inactive
# * debug
#      -O3 optimisation, full debugging information, assertions active
#
# In all build targets, we overwrite the default configuration to
# include "-g", allow cross compilation and switch to pedantic error
# reporting.
#
# You can change the build target, by adding the parameter
#   -DCMAKE_BUILD_TYPE=type
# to the cmake call.

cmake_minimum_required(VERSION 3.22)

enable_testing()

project(ai24-project-software
    DESCRIPTION "Project software for the AI 24/25 course."
    LANGUAGES   CXX)

# Path containing custom CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(common_cxx_flags)
include(macros)
include(options)

report_bitwidth()
set_up_build_types("Debug;Release")
set_up_options()

#######################################################################################
# Obtain Torch                                                                        #
#######################################################################################

find_package(Torch)

# If the Doxyfile exists, add a custom target to build the documentation.
if (EXISTS ${CMAKE_SOURCE_DIR}/docs/Doxyfile)
    # check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        # set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        # set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # request to configure the file
        # configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message(STATUS "Adding documentation target.")
        set(DOXYGEN_OUT ${CMAKE_SOURCE_DIR}/docs/Doxyfile)

        # note the option ALL which allows to build the docs together with the application
        add_custom_target(docs ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM )
    else (DOXYGEN_FOUND)
        message(NOTICE "Documentation target not added because doxygen is not installed.")
    endif (DOXYGEN_FOUND)
else ()
    message(STATUS "Documentation target not added because Doxyfile is missing.")
endif()

# Add components as a subproject.
add_subdirectory(src)
