#include "BaseRepositoryFixture.hpp"
#include "Commit.hpp"
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

    auto lastCommitHash = commits.getHeadCommitHash();
    auto commit = commits.getCommitInfo(lastCommitHash);
    EXPECT_EQ(commit.getMessage(), "Initial commit");
}

TEST_F(CommitsTests, createCommit_Empty_withParent)
{
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    ASSERT_TRUE(commits.hasAnyCommits());

    auto firstCommitHash = commits.getHeadCommitHash();
    auto commit = commits.getCommitInfo(firstCommitHash);
    EXPECT_EQ(commit.getMessage(), "Initial commit");

    commits.createCommit("Second commit");

    auto secondCommitHash = commits.getHeadCommitHash();
    commit = commits.getCommitInfo(secondCommitHash);
    EXPECT_EQ(commit.getMessage(), "Second commit");
    EXPECT_EQ(commit.getParents().size(), 1);
    EXPECT_EQ(commit.getParents()[0], firstCommitHash);
}
