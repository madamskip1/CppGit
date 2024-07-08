SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

include(FetchContent)
include(GoogleTest)

# Download and build gtest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)


add_executable(CppGit_tests)

target_sources(CppGit_tests
    PRIVATE
        CommitParser_tests.cpp
)

target_link_libraries(CppGit_tests
    PRIVATE
        CppGit::CppGit
        GTest::gtest_main
)

add_test(
    NAME CppGit_tests
    COMMAND CppGit_tests
)

gtest_discover_tests(
    CppGit_tests
)