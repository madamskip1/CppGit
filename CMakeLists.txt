cmake_minimum_required(VERSION 3.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(CppGit)

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
        src/CommitsHistory.cpp
        src/Diff.cpp
        src/Merge.cpp
        src/CherryPick.cpp


        # _details (internal)

        src/_details/Parser/CommitParser.cpp
        src/_details/Parser/BranchesParser.cpp
        src/_details/Parser/IndexParser.cpp
        src/_details/Parser/DiffParser.cpp

        src/_details/GitCommandExecutor/GitCommandExecutorUnix.cpp

        src/_details/CreateCommit.cpp
        src/_details/ThreeWayMerge.cpp
        src/_details/Refs.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

add_subdirectory(tests)