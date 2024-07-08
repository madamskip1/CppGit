cmake_minimum_required(VERSION 3.0)

project(CppGit)


add_library(CppGit)
add_library(CppGit::CppGit ALIAS CppGit)

target_sources(CppGit
    PRIVATE
        src/GitCommandExecutor.cpp
        src/Repository.cpp
        src/Commit.cpp
        src/CommitParser.cpp
)

target_include_directories(CppGit
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

add_executable(CppGit_temp_executable)

target_sources(CppGit_temp_executable
    PRIVATE
        src/main.cpp
)

target_link_libraries(CppGit_temp_executable
    PRIVATE
        CppGit
)


add_subdirectory(tests)