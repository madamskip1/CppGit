#include "BaseRepositoryFixture.hpp"
#include "Commits.hpp"

#include <gtest/gtest.h>

class CommitsTests : public BaseRepositoryFixture
{
};

TEST_F(CommitsTests, emptyRepo_checkIfThereAreAnyCommits)
{
    auto commits = repository->Commits();

    ASSERT_FALSE(commits.hasAnyCommits());
}

TEST_F(CommitsTests, createCommit_Empty)
{
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    ASSERT_TRUE(commits.hasAnyCommits());
}
