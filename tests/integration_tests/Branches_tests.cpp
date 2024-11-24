#include "BaseRepositoryFixture.hpp"
#include "Branch.hpp"
#include "Branches.hpp"
#include "Commits.hpp"
#include "Index.hpp"

#include <filesystem>
#include <gtest/gtest.h>

class BranchesTests : public BaseRepositoryFixture
{
public:
    void SetUp() override
    {
        BaseRepositoryFixture::SetUp();
        const auto& commits = repository->Commits();
        initialCommitHash = commits.createCommit("Initial commit");
    }

protected:
    std::string initialCommitHash;
};

TEST_F(BranchesTests, branchesAfterInitialCommit_getAllBranches)
{
    const auto& branches = repository->Branches();

    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, branchesAfterInitialCommit_getCurrentBranchRef)
{
    const auto& branches = repository->Branches();

    auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, branchesAfterInitialCommit_getLocalBranches)
{
    const auto& branches = repository->Branches();

    const auto allBranches = branches.getLocalBranches();
    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, branchesAfterInitialCommit_getRemoteBranches)
{
    const auto& branches = repository->Branches();

    const auto allBranches = branches.getRemoteBranches();
    ASSERT_EQ(allBranches.size(), 0);
}

TEST_F(BranchesTests, createLocalBranch_fullName)
{
    const auto& branches = repository->Branches();


    branches.createBranch("refs/heads/new_branch");


    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    const auto hashBranchRefersTo = branches.getHashBranchRefersTo("refs/heads/new_branch");
    EXPECT_EQ(hashBranchRefersTo, initialCommitHash);
}

TEST_F(BranchesTests, createLocalBranch_shortName)
{
    const auto& branches = repository->Branches();


    branches.createBranch("new_branch");


    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    const auto hashBranchRefersTo = branches.getHashBranchRefersTo("new_branch");
    EXPECT_EQ(hashBranchRefersTo, initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromBranch_fullName)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();


    branches.createBranch("new_branch");
    auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranch("refs/heads/new_branch_from_new", "refs/heads/new_branch");


    auto allBranches = branches.getAllBranches();
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
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();


    branches.createBranch("new_branch");
    auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranch("new_branch_from_new", "refs/heads/new_branch");


    auto allBranches = branches.getAllBranches();
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
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();


    branches.createBranch("new_branch");
    auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranch("refs/heads/new_branch_from_new", "new_branch");


    auto allBranches = branches.getAllBranches();
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
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();


    auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranch("refs/heads/new_branch_from_commit", initialCommitHash);


    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch_from_commit");
    EXPECT_EQ(branches.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("refs/heads/new_branch_from_commit"), initialCommitHash);
}

TEST_F(BranchesTests, deleteBranch_fullName)
{
    const auto& branches = repository->Branches();


    branches.createBranch("new_branch");
    branches.deleteBranch("refs/heads/new_branch");


    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, deleteBranch_shortName)
{
    const auto& branches = repository->Branches();


    branches.createBranch("new_branch");
    branches.deleteBranch("new_branch");


    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, getCurrentBranchHashRefsTo_fullName)
{
    const auto& branches = repository->Branches();

    auto hash = branches.getHashBranchRefersTo("refs/heads/main");
    EXPECT_EQ(hash, initialCommitHash);
}

TEST_F(BranchesTests, getCurrentBranchHashRefsTo_shortName)
{
    const auto& branches = repository->Branches();

    auto hash = branches.getHashBranchRefersTo("main");
    EXPECT_EQ(hash, initialCommitHash);
}


TEST_F(BranchesTests, currentBranchInfo)
{
    const auto& branches = repository->Branches();


    const auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    EXPECT_EQ(currentBranch.getUpstreamPull(), "");
    EXPECT_EQ(currentBranch.getUpstreamPush(), "");
    EXPECT_TRUE(currentBranch.isLocalBranch());
}

TEST_F(BranchesTests, changeBranch_shortName)
{
    const auto& branches = repository->Branches();


    branches.createBranch("new_branch");
    branches.changeCurrentBranch("new_branch");


    auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/new_branch");
}

TEST_F(BranchesTests, changeBranch_fullName)
{
    const auto& branches = repository->Branches();


    branches.createBranch("new_branch");
    branches.changeCurrentBranch("refs/heads/new_branch");

    auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/new_branch");
}


TEST_F(BranchesTests, changeBranch_shouldDeleteFile)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    const auto& index = repository->Index();


    branches.createBranch("new_branch");
    branches.changeCurrentBranch("new_branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Added new file");

    branches.changeCurrentBranch("main");


    auto currentBranch = branches.getCurrentBranchInfo();
    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file.txt"));
}

TEST_F(BranchesTests, changeBranch_shouldCreateFile)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    const auto& index = repository->Index();


    branches.createBranch("new_branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Added new file");

    branches.changeCurrentBranch("new_branch");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file.txt"));
    branches.changeCurrentBranch("main");


    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));
}

TEST_F(BranchesTests, changeBranch_shouldChangeFileContent)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    const auto& index = repository->Index();


    branches.createBranch("new_branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Main branch");
    index.add("file.txt");
    commits.createCommit("Added new file");

    branches.changeCurrentBranch("new_branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! New branch");
    index.add("file.txt");
    commits.createCommit("Changed file content");

    branches.changeCurrentBranch("main");


    EXPECT_EQ(getFileContent(repositoryPath / "file.txt"), "Hello, World! Main branch");
}

TEST_F(BranchesTests, changeBranch_shouldKeepUntrackedFile)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    const auto& index = repository->Index();


    branches.createBranch("new_branch");

    createOrOverwriteFile(repositoryPath / "tracked.txt", "");
    index.add("tracked.txt");
    commits.createCommit("Added tracked file");

    createOrOverwriteFile(repositoryPath / "untracked.txt", "");

    branches.changeCurrentBranch("new_branch");


    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "tracked.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "untracked.txt"));
}

TEST_F(BranchesTests, changeBranch_shouldFailIfWorktreeDirty)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    const auto& index = repository->Index();


    branches.createBranch("new_branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    commits.createCommit("Added new file");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");


    ASSERT_THROW(branches.changeCurrentBranch("new_branch"), std::runtime_error);
}

TEST_F(BranchesTests, detachHead)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();


    auto initialCommitHash = commits.createCommit("Initial commit");
    auto secondCommitHash = commits.createCommit("Second commit");
    branches.detachHead(initialCommitHash);


    EXPECT_EQ(getFileContent(repository->getGitDirectoryPath() / "HEAD"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromDetachedHead)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();


    auto initialCommitHash = commits.createCommit("Initial commit");
    auto secondCommitHash = commits.createCommit("Second commit");
    branches.detachHead(initialCommitHash);
    branches.createBranch("new_branch");


    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch"), initialCommitHash);
}
