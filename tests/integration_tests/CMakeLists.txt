add_executable(CppGit_integration_tests)

target_sources(CppGit_integration_tests
    PRIVATE
        BaseRepositoryFixture.hpp
        BaseRepositoryFixture.cpp
        
        InitRepository_tests.cpp
        Index_tests.cpp
        Commits_tests.cpp
        CommitsHistory_tests.cpp
        Branches_tests.cpp
)

target_link_libraries(CppGit_integration_tests
    PRIVATE
        CppGit::CppGit
        GTest::gtest_main
)

add_test(
    NAME CppGit_integration_tests
    COMMAND CppGit_integration_tests
)

gtest_discover_tests(
    CppGit_integration_tests
)