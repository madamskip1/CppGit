#include "BaseRepositoryFixture.hpp"

#include <CppGit/Branch.hpp>
#include <CppGit/BranchesManager.hpp>
#include <CppGit/CommitsManager.hpp>
#include <CppGit/IndexManager.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <filesystem>
#include <gtest/gtest.h>

class BranchesTests : public BaseRepositoryFixture
{
public:
    void SetUp() override
    {
        BaseRepositoryFixture::SetUp();
        auto commitsManager = repository->CommitsManager();
        initialCommitHash = commitsManager.createCommit("Initial commit");
    }

protected:
    std::string initialCommitHash;
};

TEST_F(BranchesTests, branchesAfterInitialCommit_getAllBranches)
{
    const auto branchesManager = repository->BranchesManager();

    const auto allBranches = branchesManager.getAllBranches();

    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, branchesAfterInitialCommit_getCurrentBranchRef)
{
    const auto branchesManager = repository->BranchesManager();

    const auto currentBranch = branchesManager.getCurrentBranchInfo();

    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, branchesAfterInitialCommit_getLocalBranches)
{
    const auto branchesManager = repository->BranchesManager();

    const auto allBranches = branchesManager.getLocalBranches();

    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, branchesAfterInitialCommit_getRemoteBranches)
{
    const auto branchesManager = repository->BranchesManager();

    const auto allBranches = branchesManager.getRemoteBranches();

    ASSERT_EQ(allBranches.size(), 0);
}

TEST_F(BranchesTests, createLocalBranch_fullName)
{
    const auto branchesManager = repository->BranchesManager();


    branchesManager.createBranch("refs/heads/new_branch");


    const auto allBranches = branchesManager.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    const auto currentBranch = branchesManager.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("refs/heads/new_branch"), initialCommitHash);
}

TEST_F(BranchesTests, createLocalBranch_shortName)
{
    const auto branchesManager = repository->BranchesManager();


    branchesManager.createBranch("new_branch");


    const auto allBranches = branchesManager.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    const auto currentBranch = branchesManager.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("new_branch"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromBranch_fullName)
{
    const auto branchesManager = repository->BranchesManager();
    const auto commitsManager = repository->CommitsManager();


    branchesManager.createBranch("new_branch");
    const auto secondCommitHash = commitsManager.createCommit("second commit");
    branchesManager.createBranch("refs/heads/new_branch_from_new", "refs/heads/new_branch");


    const auto allBranches = branchesManager.getAllBranches();
    ASSERT_EQ(allBranches.size(), 3);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(allBranches[2].getRefName(), "refs/heads/new_branch_from_new");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("new_branch"), initialCommitHash);
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("new_branch_from_new"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromBranch_shortName)
{
    const auto branchesManager = repository->BranchesManager();
    const auto commitsManager = repository->CommitsManager();


    branchesManager.createBranch("new_branch");
    const auto secondCommitHash = commitsManager.createCommit("second commit");
    branchesManager.createBranch("new_branch_from_new", "refs/heads/new_branch");


    const auto allBranches = branchesManager.getAllBranches();
    ASSERT_EQ(allBranches.size(), 3);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(allBranches[2].getRefName(), "refs/heads/new_branch_from_new");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("new_branch"), initialCommitHash);
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("new_branch_from_new"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromBranch_shortRefName)
{
    const auto branchesManager = repository->BranchesManager();
    auto commitsManager = repository->CommitsManager();


    branchesManager.createBranch("new_branch");
    const auto secondCommitHash = commitsManager.createCommit("second commit");
    branchesManager.createBranch("refs/heads/new_branch_from_new", "new_branch");


    const auto allBranches = branchesManager.getAllBranches();
    ASSERT_EQ(allBranches.size(), 3);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(allBranches[2].getRefName(), "refs/heads/new_branch_from_new");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("new_branch"), initialCommitHash);
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("new_branch_from_new"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromCommitHash)
{
    const auto branchesManager = repository->BranchesManager();
    const auto commitsManager = repository->CommitsManager();


    const auto secondCommitHash = commitsManager.createCommit("second commit");
    branchesManager.createBranch("refs/heads/new_branch_from_commit", initialCommitHash);


    const auto allBranches = branchesManager.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch_from_commit");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branchesManager.getHashBranchRefersTo("refs/heads/new_branch_from_commit"), initialCommitHash);
}

TEST_F(BranchesTests, deleteBranch_fullName)
{
    const auto branchesManager = repository->BranchesManager();


    branchesManager.createBranch("new_branch");
    branchesManager.deleteBranch("refs/heads/new_branch");


    const auto allBranches = branchesManager.getAllBranches();
    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, deleteBranch_shortName)
{
    const auto branchesManager = repository->BranchesManager();


    branchesManager.createBranch("new_branch");
    branchesManager.deleteBranch("new_branch");


    const auto allBranches = branchesManager.getAllBranches();
    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, getCurrentBranchHashRefsTo_fullName)
{
    const auto branchesManager = repository->BranchesManager();

    const auto hash = branchesManager.getHashBranchRefersTo("refs/heads/main");
    EXPECT_EQ(hash, initialCommitHash);
}

TEST_F(BranchesTests, getCurrentBranchHashRefsTo_shortName)
{
    const auto branchesManager = repository->BranchesManager();

    const auto hash = branchesManager.getHashBranchRefersTo("main");
    EXPECT_EQ(hash, initialCommitHash);
}


TEST_F(BranchesTests, currentBranchInfo)
{
    const auto branchesManager = repository->BranchesManager();


    const auto currentBranch = branchesManager.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    EXPECT_EQ(currentBranch.getUpstreamPull(), "");
    EXPECT_EQ(currentBranch.getUpstreamPush(), "");
    EXPECT_TRUE(currentBranch.isLocalBranch());
}

TEST_F(BranchesTests, changeBranch_shortName)
{
    const auto branchesManager = repository->BranchesManager();


    branchesManager.createBranch("new_branch");
    branchesManager.changeBranch("new_branch");


    const auto currentBranch = branchesManager.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/new_branch");
}

TEST_F(BranchesTests, changeBranch_fullName)
{
    const auto branchesManager = repository->BranchesManager();


    branchesManager.createBranch("new_branch");
    branchesManager.changeBranch("refs/heads/new_branch");


    const auto currentBranch = branchesManager.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/new_branch");
}


TEST_F(BranchesTests, changeBranch_shouldDeleteFile)
{
    const auto branchesManager = repository->BranchesManager();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();


    branchesManager.createBranch("new_branch");
    branchesManager.changeBranch("new_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Added new file");

    branchesManager.changeBranch("main");


    const auto currentBranch = branchesManager.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file.txt"));
}

TEST_F(BranchesTests, changeBranch_shouldCreateFile)
{
    const auto branchesManager = repository->BranchesManager();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();


    branchesManager.createBranch("new_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Added new file");

    branchesManager.changeBranch("new_branch");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file.txt"));
    branchesManager.changeBranch("main");


    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));
}

TEST_F(BranchesTests, changeBranch_shouldChangeFileContent)
{
    const auto branchesManager = repository->BranchesManager();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();


    branchesManager.createBranch("new_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Main branch");
    indexManager.add("file.txt");
    commitsManager.createCommit("Added new file");

    branchesManager.changeBranch("new_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! New branch");
    indexManager.add("file.txt");
    commitsManager.createCommit("Changed file content");

    branchesManager.changeBranch("main");


    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Main branch");
}

TEST_F(BranchesTests, changeBranch_shouldKeepUntrackedFile)
{
    const auto branchesManager = repository->BranchesManager();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();


    branchesManager.createBranch("new_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "tracked.txt", "");
    indexManager.add("tracked.txt");
    commitsManager.createCommit("Added tracked file");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "untracked.txt", "");

    branchesManager.changeBranch("new_branch");


    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "tracked.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "untracked.txt"));
}

TEST_F(BranchesTests, detachHead)
{
    const auto branchesManager = repository->BranchesManager();
    const auto commitsManager = repository->CommitsManager();


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    commitsManager.createCommit("Second commit");

    branchesManager.detachHead(initialCommitHash);


    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repository->getGitDirectoryPath() / "HEAD"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromDetachedHead)
{
    const auto branchesManager = repository->BranchesManager();
    const auto commitsManager = repository->CommitsManager();


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    commitsManager.createCommit("Second commit");
    branchesManager.detachHead(initialCommitHash);
    branchesManager.createBranch("new_branch");


    EXPECT_EQ(branchesManager.getHashBranchRefersTo("new_branch"), initialCommitHash);
}

TEST_F(BranchesTests, getCurrentBranchNameOrDetachedHash_whenHeadPointingToBranch)
{
    const auto branchesManager = repository->BranchesManager();
    auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    const auto currentBranchNameOrDetachedHash = branchesManager.getCurrentBranchNameOrDetachedHash();


    EXPECT_EQ(currentBranchNameOrDetachedHash, "refs/heads/main");
}

TEST_F(BranchesTests, getCurrentBranchNameOrDetachedHash_whenHeadPointingToCommit)
{
    const auto branchesManager = repository->BranchesManager();
    auto commitsManager = repository->CommitsManager();


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.detachHead(initialCommitHash);
    const auto currentBranchNameOrDetachedHash = branchesManager.getCurrentBranchNameOrDetachedHash();


    EXPECT_EQ(currentBranchNameOrDetachedHash, initialCommitHash);
}
