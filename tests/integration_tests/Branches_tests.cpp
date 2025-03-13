#include "BaseRepositoryFixture.hpp"

#include <CppGit/Branch.hpp>
#include <CppGit/Branches.hpp>
#include <CppGit/Commits.hpp>
#include <CppGit/Index.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <filesystem>
#include <gtest/gtest.h>

class BranchesTests : public BaseRepositoryFixture
{
public:
    void SetUp() override
    {
        BaseRepositoryFixture::SetUp();
        auto commits = repository->Commits();
        initialCommitHash = commits.createCommit("Initial commit");
    }

protected:
    std::string initialCommitHash;
};

TEST_F(BranchesTests, branchesAfterInitialCommit_getAllBranches)
{
    const auto branches = repository->Branches();

    const auto allBranches = branches.getAllBranches();

    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, branchesAfterInitialCommit_getCurrentBranchRef)
{
    const auto branches = repository->Branches();

    const auto currentBranch = branches.getCurrentBranchInfo();

    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, branchesAfterInitialCommit_getLocalBranches)
{
    const auto branches = repository->Branches();

    const auto allBranches = branches.getLocalBranches();

    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, branchesAfterInitialCommit_getRemoteBranches)
{
    const auto branches = repository->Branches();

    const auto allBranches = branches.getRemoteBranches();

    ASSERT_EQ(allBranches.size(), 0);
}

TEST_F(BranchesTests, createLocalBranch_fullName)
{
    const auto branches = repository->Branches();


    branches.createBranch("refs/heads/new_branch");


    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    const auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    EXPECT_EQ(branches.getHashBranchRefersTo("refs/heads/new_branch"), initialCommitHash);
}

TEST_F(BranchesTests, createLocalBranch_shortName)
{
    const auto branches = repository->Branches();


    branches.createBranch("new_branch");


    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    const auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromBranch_fullName)
{
    const auto branches = repository->Branches();
    const auto commits = repository->Commits();


    branches.createBranch("new_branch");
    const auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranch("refs/heads/new_branch_from_new", "refs/heads/new_branch");


    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 3);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(allBranches[2].getRefName(), "refs/heads/new_branch_from_new");
    EXPECT_EQ(branches.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch"), initialCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch_from_new"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromBranch_shortName)
{
    const auto branches = repository->Branches();
    const auto commits = repository->Commits();


    branches.createBranch("new_branch");
    const auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranch("new_branch_from_new", "refs/heads/new_branch");


    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 3);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(allBranches[2].getRefName(), "refs/heads/new_branch_from_new");
    EXPECT_EQ(branches.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch"), initialCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch_from_new"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromBranch_shortRefName)
{
    const auto branches = repository->Branches();
    auto commits = repository->Commits();


    branches.createBranch("new_branch");
    const auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranch("refs/heads/new_branch_from_new", "new_branch");


    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 3);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(allBranches[2].getRefName(), "refs/heads/new_branch_from_new");
    EXPECT_EQ(branches.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch"), initialCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch_from_new"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromCommitHash)
{
    const auto branches = repository->Branches();
    const auto commits = repository->Commits();


    const auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranch("refs/heads/new_branch_from_commit", initialCommitHash);


    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch_from_commit");
    EXPECT_EQ(branches.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("refs/heads/new_branch_from_commit"), initialCommitHash);
}

TEST_F(BranchesTests, deleteBranch_fullName)
{
    const auto branches = repository->Branches();


    branches.createBranch("new_branch");
    branches.deleteBranch("refs/heads/new_branch");


    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, deleteBranch_shortName)
{
    const auto branches = repository->Branches();


    branches.createBranch("new_branch");
    branches.deleteBranch("new_branch");


    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, getCurrentBranchHashRefsTo_fullName)
{
    const auto branches = repository->Branches();

    const auto hash = branches.getHashBranchRefersTo("refs/heads/main");
    EXPECT_EQ(hash, initialCommitHash);
}

TEST_F(BranchesTests, getCurrentBranchHashRefsTo_shortName)
{
    const auto branches = repository->Branches();

    const auto hash = branches.getHashBranchRefersTo("main");
    EXPECT_EQ(hash, initialCommitHash);
}


TEST_F(BranchesTests, currentBranchInfo)
{
    const auto branches = repository->Branches();


    const auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    EXPECT_EQ(currentBranch.getUpstreamPull(), "");
    EXPECT_EQ(currentBranch.getUpstreamPush(), "");
    EXPECT_TRUE(currentBranch.isLocalBranch());
}

TEST_F(BranchesTests, changeBranch_shortName)
{
    const auto branches = repository->Branches();


    branches.createBranch("new_branch");
    branches.changeCurrentBranch("new_branch");


    const auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/new_branch");
}

TEST_F(BranchesTests, changeBranch_fullName)
{
    const auto branches = repository->Branches();


    branches.createBranch("new_branch");
    branches.changeCurrentBranch("refs/heads/new_branch");


    const auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/new_branch");
}


TEST_F(BranchesTests, changeBranch_shouldDeleteFile)
{
    const auto branches = repository->Branches();
    const auto commits = repository->Commits();
    const auto index = repository->Index();


    branches.createBranch("new_branch");
    branches.changeCurrentBranch("new_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Added new file");

    branches.changeCurrentBranch("main");


    const auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file.txt"));
}

TEST_F(BranchesTests, changeBranch_shouldCreateFile)
{
    const auto branches = repository->Branches();
    const auto commits = repository->Commits();
    const auto index = repository->Index();


    branches.createBranch("new_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Added new file");

    branches.changeCurrentBranch("new_branch");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file.txt"));
    branches.changeCurrentBranch("main");


    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));
}

TEST_F(BranchesTests, changeBranch_shouldChangeFileContent)
{
    const auto branches = repository->Branches();
    const auto commits = repository->Commits();
    const auto index = repository->Index();


    branches.createBranch("new_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Main branch");
    index.add("file.txt");
    commits.createCommit("Added new file");

    branches.changeCurrentBranch("new_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! New branch");
    index.add("file.txt");
    commits.createCommit("Changed file content");

    branches.changeCurrentBranch("main");


    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Main branch");
}

TEST_F(BranchesTests, changeBranch_shouldKeepUntrackedFile)
{
    const auto branches = repository->Branches();
    const auto commits = repository->Commits();
    const auto index = repository->Index();


    branches.createBranch("new_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "tracked.txt", "");
    index.add("tracked.txt");
    commits.createCommit("Added tracked file");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "untracked.txt", "");

    branches.changeCurrentBranch("new_branch");


    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "tracked.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "untracked.txt"));
}

TEST_F(BranchesTests, detachHead)
{
    const auto branches = repository->Branches();
    const auto commits = repository->Commits();


    const auto initialCommitHash = commits.createCommit("Initial commit");
    commits.createCommit("Second commit");

    branches.detachHead(initialCommitHash);


    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repository->getGitDirectoryPath() / "HEAD"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromDetachedHead)
{
    const auto branches = repository->Branches();
    const auto commits = repository->Commits();


    const auto initialCommitHash = commits.createCommit("Initial commit");
    commits.createCommit("Second commit");
    branches.detachHead(initialCommitHash);
    branches.createBranch("new_branch");


    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch"), initialCommitHash);
}

TEST_F(BranchesTests, getCurrentBranchNameOrDetachedHash_whenHeadPointingToBranch)
{
    const auto branches = repository->Branches();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    const auto currentBranchNameOrDetachedHash = branches.getCurrentBranchNameOrDetachedHash();


    EXPECT_EQ(currentBranchNameOrDetachedHash, "refs/heads/main");
}

TEST_F(BranchesTests, getCurrentBranchNameOrDetachedHash_whenHeadPointingToCommit)
{
    const auto branches = repository->Branches();
    auto commits = repository->Commits();


    const auto initialCommitHash = commits.createCommit("Initial commit");
    branches.detachHead(initialCommitHash);
    const auto currentBranchNameOrDetachedHash = branches.getCurrentBranchNameOrDetachedHash();


    EXPECT_EQ(currentBranchNameOrDetachedHash, initialCommitHash);
}
