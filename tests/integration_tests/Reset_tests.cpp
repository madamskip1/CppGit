#include "BaseRepositoryFixture.hpp"
#include "Branches.hpp"
#include "Commits.hpp"
#include "Index.hpp"
#include "Reset.hpp"
#include "_details/FileUtility.hpp"

#include <gtest/gtest.h>


class ResetTests : public BaseRepositoryFixture
{
};

TEST_F(ResetTests, resetSoft)
{
    auto commits = repository->Commits();
    auto reset = repository->Reset();
    auto index = repository->Index();
    auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    reset.resetSoft(initialCommitHash);


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHard)
{
    auto commits = repository->Commits();
    auto reset = repository->Reset();
    auto index = repository->Index();
    auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    reset.resetHard(initialCommitHash);


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoftWithUntrackedFile)
{
    auto commits = repository->Commits();
    auto reset = repository->Reset();
    auto index = repository->Index();
    auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
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
    auto commits = repository->Commits();
    auto reset = repository->Reset();
    auto index = repository->Index();
    auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
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
    auto commits = repository->Commits();
    auto reset = repository->Reset();
    auto index = repository->Index();
    auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    reset.resetSoft("HEAD^");


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHard_usingHEADRelativeTildeNotation)
{
    auto commits = repository->Commits();
    auto reset = repository->Reset();
    auto index = repository->Index();
    auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");


    reset.resetHard("HEAD^");


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoft_usingHEADRelativeCaretNotation)
{
    auto commits = repository->Commits();
    auto reset = repository->Reset();
    auto index = repository->Index();
    auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    reset.resetSoft("HEAD~1");


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoft_toHEAD)
{
    auto commits = repository->Commits();
    auto reset = repository->Reset();
    auto index = repository->Index();
    auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    reset.resetSoft("HEAD");


    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHard_toHEAD)
{
    auto commits = repository->Commits();
    auto reset = repository->Reset();
    auto index = repository->Index();
    auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    reset.resetHard("HEAD");


    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 2");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoft_detachedHEAD)
{
    auto commits = repository->Commits();
    auto reset = repository->Reset();
    auto index = repository->Index();
    auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");
    index.add("file.txt");
    commits.createCommit("Third commit");
    branches.detachHead(secondCommitHash);

    reset.resetSoft(initialCommitHash);


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_THROW(static_cast<void>(branches.getCurrentBranchName()), std::runtime_error);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 2");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHard_detachedHEAD)
{
    auto commits = repository->Commits();
    auto reset = repository->Reset();
    auto index = repository->Index();
    auto branches = repository->Branches();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");
    index.add("file.txt");
    commits.createCommit("Third commit");
    branches.detachHead(secondCommitHash);

    reset.resetHard(initialCommitHash);


    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_THROW(static_cast<void>(branches.getCurrentBranchName()), std::runtime_error);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}
