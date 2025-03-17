#include "BaseRepositoryFixture.hpp"

#include <CppGit/Commit.hpp>
#include <CppGit/Commits.hpp>
#include <CppGit/CommitsLog.hpp>
#include <CppGit/Diff.hpp>
#include <CppGit/Index.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <filesystem>
#include <gtest/gtest.h>

class CommitsTests : public BaseRepositoryFixture
{
};

TEST_F(CommitsTests, emptyRepo_checkIfThereAreAnyCommits)
{
    const auto commits = repository->Commits();

    ASSERT_FALSE(commits.hasAnyCommits());
}

TEST_F(CommitsTests, createCommit_empty)
{
    const auto commits = repository->Commits();

    const auto initialCommitHash = commits.createCommit("Initial commit");

    auto commitInfo = commits.getCommitInfo(initialCommitHash);
    ASSERT_TRUE(commits.hasAnyCommits());
    EXPECT_EQ(initialCommitHash, commits.getHeadCommitHash());
    EXPECT_EQ(commitInfo.getMessage(), "Initial commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getMessageAndDescription(), "Initial commit");
}

TEST_F(CommitsTests, createCommit_empty_withParent)
{
    const auto commits = repository->Commits();

    const auto initialCommitHash = commits.createCommit("Initial commit");
    const auto secondCommitHash = commits.createCommit("Second commit");

    const auto commitInfo = commits.getCommitInfo(secondCommitHash);
    EXPECT_EQ(secondCommitHash, commits.getHeadCommitHash());
    EXPECT_NE(secondCommitHash, initialCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Second commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getMessageAndDescription(), "Second commit");
    EXPECT_EQ(commitInfo.getParents().size(), 1);
    EXPECT_EQ(commitInfo.getParents()[0], initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
}

TEST_F(CommitsTests, createCommit_empty_withDescription)
{
    const auto commits = repository->Commits();

    const auto initialCommitHash = commits.createCommit("Initial commit", "Initial commit description");

    const auto commitInfo = commits.getCommitInfo(initialCommitHash);
    ASSERT_TRUE(commits.hasAnyCommits());
    EXPECT_EQ(initialCommitHash, commits.getHeadCommitHash());
    EXPECT_EQ(commitInfo.getMessage(), "Initial commit");
    EXPECT_EQ(commitInfo.getDescription(), "Initial commit description");
    EXPECT_EQ(commitInfo.getMessageAndDescription(), "Initial commit\n\nInitial commit description");
}

TEST_F(CommitsTests, createCommit_shouldPreserveChangesInNotAddedTrackedFiles)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    index.add("file2.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");


    const auto commit = commits.getCommitInfo(secondCommitHash);
    EXPECT_EQ(commit.getMessage(), "Second commit");
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Modified");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
}

TEST_F(CommitsTests, createCommit_shouldPreserveChangesInNotAddedUntrackedFiles)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! file2");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");


    const auto commit = commits.getCommitInfo(secondCommitHash);
    EXPECT_EQ(commit.getMessage(), "Second commit");
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World! file2");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
}

TEST_F(CommitsTests, amendCommit_noChanges)
{
    const auto commits = repository->Commits();
    auto commitsLog = repository->CommitsLog();
    commitsLog.setOrder(CppGit::CommitsLog::Order::REVERSE);


    const auto initialCommitHash = createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    const auto amendedCommitHash = commits.amendCommit();


    // even we do amend commit without changes and same message, it should create new commit (for eg. because of different committer/committer date)
    EXPECT_NE(amendedCommitHash, initialCommitHash);
    const auto log = commitsLog.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 1);
    EXPECT_EQ(log[0].getMessage(), "Initial commit");
    EXPECT_EQ(log[0].getHash(), amendedCommitHash);
    EXPECT_EQ(log[0].getDescription(), "");
    EXPECT_EQ(log[0].getMessageAndDescription(), "Initial commit");
    const auto commitInfo = commits.getCommitInfo(amendedCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Initial commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getMessageAndDescription(), "Initial commit");
    EXPECT_EQ(commitInfo.getParents().size(), 0);
    checkCommitAuthorEqualTest(commitInfo);
    checkCommitCommiterNotEqualTest(commitInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
}

TEST_F(CommitsTests, amendCommit_changeMsg)
{
    const auto commits = repository->Commits();
    auto commitsLog = repository->CommitsLog();
    commitsLog.setOrder(CppGit::CommitsLog::Order::REVERSE);


    const auto initialCommitHash = createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    const auto amendedCommitHash = commits.amendCommit("Amended commit");


    EXPECT_NE(amendedCommitHash, initialCommitHash);
    const auto log = commitsLog.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 1);
    EXPECT_EQ(log[0].getHash(), amendedCommitHash);
    EXPECT_EQ(log[0].getMessage(), "Amended commit");
    EXPECT_EQ(log[0].getDescription(), "");
    EXPECT_EQ(log[0].getMessageAndDescription(), "Amended commit");
    const auto commitInfo = commits.getCommitInfo(amendedCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Amended commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getMessageAndDescription(), "Amended commit");
    EXPECT_EQ(commitInfo.getParents().size(), 0);
    checkCommitAuthorEqualTest(commitInfo);
    checkCommitCommiterNotEqualTest(commitInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
}

TEST_F(CommitsTests, amendCommit_changeMsgWithDescription)
{
    const auto commits = repository->Commits();
    auto commitsLog = repository->CommitsLog();
    commitsLog.setOrder(CppGit::CommitsLog::Order::REVERSE);


    const auto initialCommitHash = createCommitWithTestAuthorCommiterWithoutParent("Initial commit", "Initial description");
    const auto amendedCommitHash = commits.amendCommit("Amended commit");


    EXPECT_NE(amendedCommitHash, initialCommitHash);
    const auto log = commitsLog.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 1);
    EXPECT_EQ(log[0].getHash(), amendedCommitHash);
    EXPECT_EQ(log[0].getMessage(), "Amended commit");
    EXPECT_EQ(log[0].getDescription(), "");
    EXPECT_EQ(log[0].getMessageAndDescription(), "Amended commit");
    const auto commitInfo = commits.getCommitInfo(amendedCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Amended commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getParents().size(), 0);
    checkCommitAuthorEqualTest(commitInfo);
    checkCommitCommiterNotEqualTest(commitInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
}

TEST_F(CommitsTests, amendCommit_changeMsgAndDescription)
{
    const auto commits = repository->Commits();
    auto commitsLog = repository->CommitsLog();
    commitsLog.setOrder(CppGit::CommitsLog::Order::REVERSE);


    const auto& initialCommitHash = createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    const auto amendedCommitHash = commits.amendCommit("Amended commit", "Amended description");


    EXPECT_NE(amendedCommitHash, initialCommitHash);
    const auto log = commitsLog.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 1);
    EXPECT_EQ(log[0].getHash(), amendedCommitHash);
    EXPECT_EQ(log[0].getMessage(), "Amended commit");
    EXPECT_EQ(log[0].getDescription(), "Amended description");
    EXPECT_EQ(log[0].getMessageAndDescription(), "Amended commit\n\nAmended description");
    const auto commitInfo = commits.getCommitInfo(amendedCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Amended commit");
    EXPECT_EQ(commitInfo.getDescription(), "Amended description");
    EXPECT_EQ(commitInfo.getParents().size(), 0);
    checkCommitAuthorEqualTest(commitInfo);
    checkCommitCommiterNotEqualTest(commitInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
}

TEST_F(CommitsTests, amendCommit_addFile)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();
    auto commitsLog = repository->CommitsLog();
    commitsLog.setOrder(CppGit::CommitsLog::Order::REVERSE);

    const auto initialCommitHash = commits.createCommit("Initial commit");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");

    const auto amendedCommitHash = commits.amendCommit();


    EXPECT_NE(amendedCommitHash, secondCommitHash);
    const auto log = commitsLog.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[0].getMessage(), "Initial commit");
    EXPECT_EQ(log[0].getDescription(), "");
    EXPECT_EQ(log[0].getMessageAndDescription(), "Initial commit");
    EXPECT_EQ(log[1].getHash(), amendedCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Second commit");
    EXPECT_EQ(log[1].getDescription(), "");
    EXPECT_EQ(log[1].getMessageAndDescription(), "Second commit");
    const auto commitInfo = commits.getCommitInfo(amendedCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Second commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getMessageAndDescription(), "Second commit");
    EXPECT_EQ(commitInfo.getParents().size(), 1);
    EXPECT_EQ(commitInfo.getParents()[0], initialCommitHash);
    checkCommitAuthorEqualTest(commitInfo);
    checkCommitCommiterNotEqualTest(commitInfo);
    const auto diffFiles = diff.getDiff();
    ASSERT_EQ(diffFiles.size(), 1);
    const auto& diffFile = diffFiles[0];
    EXPECT_EQ(diffFile.isCombined, false);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "file.txt");
    EXPECT_EQ(diffFile.newMode, 100'644);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(CommitsTests, amendCommit_withOneParent)
{
    const auto commits = repository->Commits();
    auto commitsLog = repository->CommitsLog();
    commitsLog.setOrder(CppGit::CommitsLog::Order::REVERSE);

    const auto initialCommitHash = commits.createCommit("Initial commit");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    const auto amendedCommitHash = commits.amendCommit();


    EXPECT_NE(amendedCommitHash, initialCommitHash);
    EXPECT_NE(amendedCommitHash, secondCommitHash);
    const auto log = commitsLog.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[0].getMessage(), "Initial commit");
    EXPECT_EQ(log[0].getDescription(), "");
    EXPECT_EQ(log[0].getMessageAndDescription(), "Initial commit");
    EXPECT_EQ(log[1].getHash(), amendedCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Second commit");
    EXPECT_EQ(log[1].getDescription(), "");
    EXPECT_EQ(log[1].getMessageAndDescription(), "Second commit");
    const auto commitInfo = commits.getCommitInfo(amendedCommitHash);
    EXPECT_EQ(commitInfo.getMessage(), "Second commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getMessageAndDescription(), "Second commit");
    EXPECT_EQ(commitInfo.getDescription(), "");
    EXPECT_EQ(commitInfo.getParents().size(), 1);
    EXPECT_EQ(commitInfo.getParents()[0], initialCommitHash);
    checkCommitAuthorEqualTest(commitInfo);
    checkCommitCommiterNotEqualTest(commitInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}
