#include "BaseRepositoryFixture.hpp"
#include "Commit.hpp"
#include "Commits.hpp"
#include "Index.hpp"
#include "_details/CreateCommit.hpp"
#include "_details/FileUtility.hpp"

#include <filesystem>
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


    auto initialCommitHash = commits.createCommit("Initial commit");

    auto commitInfo = commits.getCommitInfo(initialCommitHash);
    ASSERT_TRUE(commits.hasAnyCommits());
    EXPECT_EQ(initialCommitHash, commits.getHeadCommitHash());
    EXPECT_EQ(commitInfo.getMessage(), "Initial commit");
}

TEST_F(CommitsTests, createCommit_empty_withParent)
{
    auto commits = repository->Commits();

    auto initialCommitHash = commits.createCommit("Initial commit");
    auto secondCommitHash = commits.createCommit("Second commit");

    auto commitInfo = commits.getCommitInfo(secondCommitHash);
    EXPECT_EQ(secondCommitHash, commits.getHeadCommitHash());
    EXPECT_NE(secondCommitHash, initialCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Second commit");
    EXPECT_EQ(commitInfo.getParents().size(), 1);
    EXPECT_EQ(commitInfo.getParents()[0], initialCommitHash);
}

TEST_F(CommitsTests, createCommit_empty_withDescription)
{
    auto commits = repository->Commits();


    auto initialCommitHash = commits.createCommit("Initial commit", "Initial commit description");


    auto commitInfo = commits.getCommitInfo(initialCommitHash);
    ASSERT_TRUE(commits.hasAnyCommits());
    EXPECT_EQ(initialCommitHash, commits.getHeadCommitHash());
    EXPECT_EQ(commitInfo.getMessage(), "Initial commit");
    EXPECT_EQ(commitInfo.getDescription(), "Initial commit description");
}

TEST_F(CommitsTests, createCommit_shouldPreserveChangesInNotAddedTrackedFiles)
{
    auto commits = repository->Commits();
    auto index = repository->Index();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    index.add("file2.txt");
    auto secondCommitHash = commits.createCommit("Second commit");


    auto commit = commits.getCommitInfo(secondCommitHash);
    EXPECT_EQ(commit.getMessage(), "Second commit");
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Modified");
}

TEST_F(CommitsTests, createCommit_shouldPreserveChangesInNotAddedUntrackedFiles)
{
    auto commits = repository->Commits();
    auto index = repository->Index();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! file2");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");


    auto commit = commits.getCommitInfo(secondCommitHash);
    EXPECT_EQ(commit.getMessage(), "Second commit");
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World! file2");
}

TEST_F(CommitsTests, amendCommit_noCommits)
{
    auto commits = repository->Commits();

    EXPECT_THROW(commits.amendCommit(), std::runtime_error);
}

TEST_F(CommitsTests, amendCommit_noChanges)
{
    auto commits = repository->Commits();


    auto envp = prepareCommitAuthorCommiterTestEnvp();
    const auto& initialCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Initial commit", {}, envp);
    auto amendedCommitHash = commits.amendCommit();


    auto commitInfo = commits.getCommitInfo(amendedCommitHash);
    EXPECT_NE(amendedCommitHash, initialCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Initial commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getParents().size(), 0);
    checkCommitAuthorEqualTest(commitInfo);
    checkCommitCommiterNotEqualTest(commitInfo);
}

TEST_F(CommitsTests, amendCommit_changeMsg)
{
    auto commits = repository->Commits();


    auto envp = prepareCommitAuthorCommiterTestEnvp();
    const auto& initialCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Initial commit", {}, envp);
    auto amendedCommitHash = commits.amendCommit("Amended commit");


    auto commitInfo = commits.getCommitInfo(amendedCommitHash);
    EXPECT_NE(amendedCommitHash, initialCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Amended commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getParents().size(), 0);
    checkCommitAuthorEqualTest(commitInfo);
    checkCommitCommiterNotEqualTest(commitInfo);
}

TEST_F(CommitsTests, amendCommit_changeMsgWithDescription)
{
    auto commits = repository->Commits();


    auto envp = prepareCommitAuthorCommiterTestEnvp();
    const auto& initialCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Initial commit", {}, envp);
    auto amendedCommitHash = commits.amendCommit("Amended commit");


    auto commitInfo = commits.getCommitInfo(amendedCommitHash);
    EXPECT_NE(amendedCommitHash, initialCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Amended commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getParents().size(), 0);
    checkCommitAuthorEqualTest(commitInfo);
    checkCommitCommiterNotEqualTest(commitInfo);
}

TEST_F(CommitsTests, amendCommit_changeMsgAndDescription)
{
    auto commits = repository->Commits();


    auto envp = prepareCommitAuthorCommiterTestEnvp();
    const auto& initialCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Initial commit", {}, envp);
    auto amendedCommitHash = commits.amendCommit("Amended commit", "Amended description");


    auto commitInfo = commits.getCommitInfo(amendedCommitHash);
    EXPECT_NE(amendedCommitHash, initialCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Amended commit");
    EXPECT_EQ(commitInfo.getDescription(), "Amended description");
    EXPECT_EQ(commitInfo.getParents().size(), 0);
    checkCommitAuthorEqualTest(commitInfo);
    checkCommitCommiterNotEqualTest(commitInfo);
}

TEST_F(CommitsTests, amendCommit_addFile)
{
    auto commits = repository->Commits();
    auto index = repository->Index();


    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto initialCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Initial commit", {}, envp);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");

    auto amendedCommitHash = commits.amendCommit();


    auto commitInfo = commits.getCommitInfo(amendedCommitHash);
    EXPECT_NE(amendedCommitHash, initialCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Initial commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getParents().size(), 0);
    checkCommitAuthorEqualTest(commitInfo);
    checkCommitCommiterNotEqualTest(commitInfo);
}

TEST_F(CommitsTests, amendCommit_withOneParent)
{
    auto commits = repository->Commits();


    auto initialCommitHash = commits.createCommit("Initial commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommitHash }, envp);
    auto amendedCommitHash = commits.amendCommit();


    auto commitInfo = commits.getCommitInfo(amendedCommitHash);
    EXPECT_NE(amendedCommitHash, initialCommitHash);
    EXPECT_NE(amendedCommitHash, secondCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Second commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getParents().size(), 1);
    EXPECT_EQ(commitInfo.getParents()[0], initialCommitHash);
    checkCommitAuthorEqualTest(commitInfo);
    checkCommitCommiterNotEqualTest(commitInfo);
}
