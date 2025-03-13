#include "BaseRepositoryFixture.hpp"

#include <CppGit/Branches.hpp>
#include <CppGit/Commits.hpp>
#include <CppGit/Index.hpp>
#include <CppGit/Reset.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <gtest/gtest.h>


class ResetTests : public BaseRepositoryFixture
{
};

TEST_F(ResetTests, resetSoft)
{
    const auto commits = repository->Commits();
    const auto reset = repository->Reset();
    const auto index = repository->Index();
    const auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    reset.resetSoft(initialCommitHash);


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHard)
{
    const auto commits = repository->Commits();
    const auto reset = repository->Reset();
    const auto index = repository->Index();
    const auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    reset.resetHard(initialCommitHash);


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoftWithUntrackedFile)
{
    const auto commits = repository->Commits();
    const auto reset = repository->Reset();
    const auto index = repository->Index();
    const auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! 4");

    reset.resetSoft(initialCommitHash);


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World! 4");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHardWithUntrackedFile)
{
    const auto commits = repository->Commits();
    const auto reset = repository->Reset();
    const auto index = repository->Index();
    const auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! 4");

    reset.resetHard(initialCommitHash);


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World! 4");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoft_usingHEADRelativeTildeNotation)
{
    const auto commits = repository->Commits();
    const auto reset = repository->Reset();
    const auto index = repository->Index();
    const auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    reset.resetSoft("HEAD^");


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHard_usingHEADRelativeTildeNotation)
{
    const auto commits = repository->Commits();
    const auto reset = repository->Reset();
    const auto index = repository->Index();
    const auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");


    reset.resetHard("HEAD^");


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoft_usingHEADRelativeCaretNotation)
{
    const auto commits = repository->Commits();
    const auto reset = repository->Reset();
    const auto index = repository->Index();
    const auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    reset.resetSoft("HEAD~1");


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoft_toHEAD)
{
    const auto commits = repository->Commits();
    const auto reset = repository->Reset();
    const auto index = repository->Index();
    const auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    reset.resetSoft("HEAD");


    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHard_toHEAD)
{
    const auto commits = repository->Commits();
    const auto reset = repository->Reset();
    const auto index = repository->Index();
    const auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    reset.resetHard("HEAD");


    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 2");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoft_detachedHEAD)
{
    const auto commits = repository->Commits();
    const auto reset = repository->Reset();
    const auto index = repository->Index();
    const auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");
    index.add("file.txt");
    commits.createCommit("Third commit");
    branches.detachHead(secondCommitHash);

    reset.resetSoft(initialCommitHash);


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchNameOrDetachedHash(), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 2");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHard_detachedHEAD)
{
    const auto commits = repository->Commits();
    const auto reset = repository->Reset();
    const auto index = repository->Index();
    const auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");
    index.add("file.txt");
    commits.createCommit("Third commit");
    branches.detachHead(secondCommitHash);

    reset.resetHard(initialCommitHash);


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchNameOrDetachedHash(), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}
