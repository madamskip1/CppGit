cmake_minimum_required(VERSION 3.20)

project(CppGit)

if (${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_STANDARD 23)
endif()

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if (ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    set(ENV{ASAN_OPTIONS} "detect_leaks=1")
endif()

if (ENABLE_UBSAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
endif()

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        src/Repository.cpp
        src/Commit.cpp
        src/Branch.cpp
        src/Branches.cpp
        src/Index.cpp
        src/Commits.cpp
        src/CommitsLog.cpp
        src/Diff.cpp
        src/Merge.cpp
        src/CherryPick.cpp
        src/Rebase.cpp
        src/Reset.cpp

        # _details (internal)

        src/_details/Parser/CommitParser.cpp
        src/_details/Parser/BranchesParser.cpp
        src/_details/Parser/IndexParser.cpp
        src/_details/Parser/DiffParser.cpp

        src/_details/GitCommandExecutor/GitCommandExecutor.cpp
        src/_details/GitCommandExecutor/GitCommandExecutorUnix.cpp

        src/_details/CreateCommit.cpp
        src/_details/AmendCommit.cpp
        src/_details/ThreeWayMerge.cpp
        src/_details/Refs.cpp
        src/_details/IndexWorktree.cpp
        src/_details/ApplyDiff.cpp
        src/_details/RebaseFilesHelper.cpp
        src/_details/GitFilesHelper.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

target_compile_options(${PROJECT_NAME}
PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_subdirectory(tests)