#include "BaseRepositoryFixture.hpp"
#include "Commit.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"

#include <gtest/gtest.h>

class CommitsTests : public BaseRepositoryFixture
{
};

TEST_F(CommitsTests, emptyRepo_checkIfThereAreAnyCommits)
{
    auto commits = repository->Commits();

    ASSERT_FALSE(commits.hasAnyCommits());
}

TEST_F(CommitsTests, getHeadCommitHash_noCommits)
{
    auto commits = repository->Commits();

    EXPECT_THROW(commits.getHeadCommitHash(), std::runtime_error);
}

TEST_F(CommitsTests, createCommit_empty)
{
    auto commits = repository->Commits();

    auto commitHash = commits.createCommit("Initial commit");

    ASSERT_TRUE(commits.hasAnyCommits());
    EXPECT_EQ(commitHash, commits.getHeadCommitHash());

    auto commit = commits.getCommitInfo(commitHash);
    EXPECT_EQ(commit.getMessage(), "Initial commit");
}

TEST_F(CommitsTests, createCommit_empty_withParent)
{
    auto commits = repository->Commits();

    auto firstCommitHash = commits.createCommit("Initial commit");

    ASSERT_TRUE(commits.hasAnyCommits());
    EXPECT_EQ(firstCommitHash, commits.getHeadCommitHash());

    auto commit = commits.getCommitInfo(firstCommitHash);
    EXPECT_EQ(commit.getMessage(), "Initial commit");

    auto secondCommitHash = commits.createCommit("Second commit");

    EXPECT_EQ(secondCommitHash, commits.getHeadCommitHash());
    commit = commits.getCommitInfo(secondCommitHash);
    EXPECT_EQ(commit.getMessage(), "Second commit");
    EXPECT_EQ(commit.getParents().size(), 1);
    EXPECT_EQ(commit.getParents()[0], firstCommitHash);
}

TEST_F(CommitsTests, createCommit_empty_withDescription)
{
    auto commits = repository->Commits();

    auto commitHash = commits.createCommit("Initial commit", "Initial commit description");

    ASSERT_TRUE(commits.hasAnyCommits());
    EXPECT_EQ(commitHash, commits.getHeadCommitHash());

    auto commit = commits.getCommitInfo(commitHash);
    EXPECT_EQ(commit.getMessage(), "Initial commit");
    EXPECT_EQ(commit.getDescription(), "Initial commit description");
}
