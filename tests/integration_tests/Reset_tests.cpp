#include "BaseRepositoryFixture.hpp"

#include <CppGit/BranchesManager.hpp>
#include <CppGit/CommitsManager.hpp>
#include <CppGit/IndexManager.hpp>
#include <CppGit/Resetter.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <gtest/gtest.h>


class ResetTests : public BaseRepositoryFixture
{
};

TEST_F(ResetTests, resetSoft)
{
    const auto resetter = repository->Resetter();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    resetter.resetSoft(initialCommitHash);


    EXPECT_EQ(commitsManager.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branchesManager.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHard)
{
    const auto resetter = repository->Resetter();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    resetter.resetHard(initialCommitHash);


    EXPECT_EQ(commitsManager.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branchesManager.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoftWithUntrackedFile)
{
    const auto resetter = repository->Resetter();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! 4");

    resetter.resetSoft(initialCommitHash);


    EXPECT_EQ(commitsManager.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branchesManager.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World! 4");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHardWithUntrackedFile)
{
    const auto resetter = repository->Resetter();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! 4");

    resetter.resetHard(initialCommitHash);


    EXPECT_EQ(commitsManager.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branchesManager.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World! 4");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoft_usingHEADRelativeTildeNotation)
{
    const auto resetter = repository->Resetter();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    resetter.resetSoft("HEAD^");


    EXPECT_EQ(commitsManager.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branchesManager.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHard_usingHEADRelativeTildeNotation)
{
    const auto resetter = repository->Resetter();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");


    resetter.resetHard("HEAD^");


    EXPECT_EQ(commitsManager.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branchesManager.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoft_usingHEADRelativeCaretNotation)
{
    const auto resetter = repository->Resetter();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    resetter.resetSoft("HEAD~1");


    EXPECT_EQ(commitsManager.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branchesManager.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoft_toHEAD)
{
    const auto resetter = repository->Resetter();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    resetter.resetSoft("HEAD");


    EXPECT_EQ(commitsManager.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(branchesManager.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 3");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHard_toHEAD)
{
    const auto resetter = repository->Resetter();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");

    resetter.resetHard("HEAD");


    EXPECT_EQ(commitsManager.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(branchesManager.getCurrentBranchName(), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 2");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetSoft_detachedHEAD)
{
    const auto resetter = repository->Resetter();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");
    indexManager.add("file.txt");
    commitsManager.createCommit("Third commit");
    branchesManager.detachHead(secondCommitHash);

    resetter.resetSoft(initialCommitHash);


    EXPECT_EQ(commitsManager.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branchesManager.getCurrentBranchNameOrDetachedHash(), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! 2");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(ResetTests, resetHard_detachedHEAD)
{
    const auto resetter = repository->Resetter();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 2");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! 3");
    indexManager.add("file.txt");
    commitsManager.createCommit("Third commit");
    branchesManager.detachHead(secondCommitHash);

    resetter.resetHard(initialCommitHash);


    EXPECT_EQ(commitsManager.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(branchesManager.getCurrentBranchNameOrDetachedHash(), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}
