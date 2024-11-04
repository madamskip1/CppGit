cmake_minimum_required(VERSION 3.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(CppGit)


add_library(CppGit)
add_library(CppGit::CppGit ALIAS CppGit)

target_sources(CppGit
    PRIVATE
        src/GitCommandExecutor/GitCommandExecutorUnix.cpp
        src/Repository.cpp
        src/Commit.cpp
        src/Branch.cpp
        src/Branches.cpp
        src/Index.cpp
        src/Commits.cpp
        src/CommitsHistory.cpp
        src/Diff.cpp
        src/Merge.cpp
        src/CherryPick.cpp

        src/Parser/CommitParser.cpp
        src/Parser/BranchesParser.cpp
        src/Parser/IndexParser.cpp
        src/Parser/DiffParser.cpp
)

target_include_directories(CppGit
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

add_subdirectory(tests)