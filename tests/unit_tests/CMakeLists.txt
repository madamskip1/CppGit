add_executable(CppGit_unit_tests)

target_sources(CppGit_unit_tests
    PRIVATE
        Parser_tests.cpp
        CommitParser_tests.cpp
        BranchesParser_tests.cpp
)

target_link_libraries(CppGit_unit_tests
    PRIVATE
        CppGit::CppGit
        GTest::gtest_main
)

add_test(
    NAME CppGit_unit_tests
    COMMAND CppGit_unit_tests
)

gtest_discover_tests(
    CppGit_unit_tests
)