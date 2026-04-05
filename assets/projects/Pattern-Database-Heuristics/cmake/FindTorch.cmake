# Download the correct version for each system
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYS_NAME)

if (${SYS_NAME} STREQUAL "darwin")
    if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm64")
        set(TORCH_URL "https://fai.cs.uni-saarland.de/ai24/libtorch-darwin-arm64.zip")
        set(TORCH_HASH ece7aaffee16bce19a467c1bd68c67f3)
    else()
        set(TORCH_URL "https://fai.cs.uni-saarland.de/ai24/libtorch-darwin.zip")
        set(TORCH_HASH 7111c02aa94be141b35f2291cb72d70f)
    endif()
elseif (${SYS_NAME} STREQUAL "windows")
    set(TORCH_URL "https://fai.cs.uni-saarland.de/ai24/libtorch-windows.zip")
    set(TORCH_HASH 7773e4940fec973b5315bbbbfc3ba68d)
elseif (${SYS_NAME} STREQUAL "linux")
    set(TORCH_URL "https://fai.cs.uni-saarland.de/ai24/libtorch-linux.zip")
    set(TORCH_HASH e6f5f0467920b017ec658599406c72f3)
else()
    message(FATAL_ERROR "Unsupported system.")
endif()

set(TORCH_DOWNLOAD_DIR "${PROJECT_SOURCE_DIR}/external/Torch")

# Try to find an existing installation first. Check the 'external' directory too.
find_package(Torch CONFIG PATHS ${TORCH_DOWNLOAD_DIR} QUIET)

# If there is none, download from our servers into the 'external' directory.
if (NOT Torch_FOUND)
    message(STATUS "Downloading PyTorch C++ library from FAI web server. This might take a while.")
    file(
        DOWNLOAD ${TORCH_URL} ${CMAKE_CURRENT_BINARY_DIR}/torch.zip
        EXPECTED_MD5 ${TORCH_HASH}
        SHOW_PROGRESS)

    file(
        ARCHIVE_EXTRACT
        INPUT ${CMAKE_CURRENT_BINARY_DIR}/torch.zip
        DESTINATION ${TORCH_DOWNLOAD_DIR})

    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/torch.zip)

    set(Torch_ROOT ${TORCH_DOWNLOAD_DIR})

    # Now try to find it again. This should not fail.
    find_package(Torch CONFIG REQUIRED)
endif()