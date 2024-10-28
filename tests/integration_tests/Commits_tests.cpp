#include "BaseRepositoryFixture.hpp"
#include "Commit.hpp"
#include "Commits.hpp"
#include "Index.hpp"

#include <filesystem>
#include <fstream>
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

TEST_F(CommitsTests, createCommit_shouldPreserveChangesInNotAddedTrackedFiles)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    index.add("file.txt");
    commits.createCommit("Initial commit");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Modified";
    file.close();

    std::ofstream secondFile = std::ofstream(repositoryPath / "file2.txt");
    secondFile << "Second file";
    secondFile.close();
    index.add("file2.txt");
    commits.createCommit("Second commit");

    auto commit = commits.getCommitInfo(commits.getHeadCommitHash());
    EXPECT_EQ(commit.getMessage(), "Second commit");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));

    std::ifstream fileRead(repositoryPath / "file.txt");
    std::ostringstream fileContentStream;
    fileContentStream << fileRead.rdbuf();
    EXPECT_EQ(fileContentStream.str(), "Hello, World! Modified");
}

TEST_F(CommitsTests, createCommit_shouldPreserveChangesInNotAddedUntrackedFiles)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    index.add("file.txt");
    commits.createCommit("Initial commit");

    std::ofstream secondFile = std::ofstream(repositoryPath / "file2.txt");
    secondFile << "Second file";
    secondFile.close();

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Modified";
    file.close();
    index.add("file.txt");
    commits.createCommit("Second commit");

    auto commit = commits.getCommitInfo(commits.getHeadCommitHash());
    EXPECT_EQ(commit.getMessage(), "Second commit");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));

    std::ifstream file2Read(repositoryPath / "file2.txt");
    std::ostringstream file2ContentStream;
    file2ContentStream << file2Read.rdbuf();
    EXPECT_EQ(file2ContentStream.str(), "Second file");
}
