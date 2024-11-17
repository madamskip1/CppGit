add_executable(${PROJECT_NAME}_unit_tests)

target_sources(${PROJECT_NAME}_unit_tests
    PRIVATE
        Parser_tests.cpp
        CommitParser_tests.cpp
        BranchesParser_tests.cpp
        IndexParser_tests.cpp
        DiffParser_tests.cpp
)

target_link_libraries(${PROJECT_NAME}_unit_tests
    PRIVATE
        ${PROJECT_NAME}::${PROJECT_NAME}
        GTest::gtest_main
)

add_test(
    NAME ${PROJECT_NAME}_unit_tests
    COMMAND ${PROJECT_NAME}_unit_tests
)

gtest_discover_tests(
    ${PROJECT_NAME}_unit_tests
)