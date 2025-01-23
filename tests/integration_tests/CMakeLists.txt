add_executable(${PROJECT_NAME}_integration_tests)

target_sources(${PROJECT_NAME}_integration_tests
    PRIVATE
        BaseRepositoryFixture.hpp
        BaseRepositoryFixture.cpp
        
        InitRepository_tests.cpp
        Index_tests.cpp
        Commits_tests.cpp
        CommitsHistory_tests.cpp
        Branches_tests.cpp
        Diff_tests.cpp
        Merge_tests.cpp
        CherryPick_tests.cpp

        Rebase_tests/Rebase_basic_tests.cpp
        Rebase_tests/Rebase_interactive_basic_tests.cpp
        Rebase_tests/Rebase_interactive_Break_tests.cpp
        Rebase_tests/Rebase_interactive_Drop_tests.cpp
        Rebase_tests/Rebase_interactive_Edit_tests.cpp
        Rebase_tests/Rebase_interactive_Fixup_tests.cpp
        Rebase_tests/Rebase_interactive_FixupSquash_tests.cpp
        Rebase_tests/Rebase_interactive_Squash_tests.cpp
        Rebase_tests/Rebase_interactive_SquashFixup_tests.cpp
        Rebase_tests/Rebase_interactive_Reword_tests.cpp
)

target_link_libraries(${PROJECT_NAME}_integration_tests
    PRIVATE
        ${PROJECT_NAME}::${PROJECT_NAME}
        GTest::gtest_main
)

add_test(
    NAME ${PROJECT_NAME}_integration_tests
    COMMAND ${PROJECT_NAME}_integration_tests
)

gtest_discover_tests(
    ${PROJECT_NAME}_integration_tests
)