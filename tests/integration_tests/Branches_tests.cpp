#include "BaseRepositoryFixture.hpp"
#include "Branch.hpp"
#include "Branches.hpp"

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
    const auto currentBranchRef = branches.getCurrentBranchRef();

    EXPECT_EQ(currentBranchRef, "refs/heads/main");
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

    const auto currentBranch = branches.getCurrentBranchRef();
    EXPECT_EQ(currentBranch, "refs/heads/main");
}

TEST_F(BranchesTests, createLocalBranch_shortName)
{
    const auto& branches = repository->Branches();
    branches.createBranch("new_branch");

    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");

    const auto currentBranch = branches.getCurrentBranchRef();
    EXPECT_EQ(currentBranch, "refs/heads/main");
}

TEST_F(BranchesTests, deleteBranch_fullName)
{
    const auto& branches = repository->Branches();
    branches.createBranch("new_branch");

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");

    branches.deleteBranch("refs/heads/new_branch");

    allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, deleteBranch_shortName)
{
    const auto& branches = repository->Branches();
    branches.createBranch("new_branch");

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");

    branches.deleteBranch("new_branch");

    allBranches = branches.getAllBranches();
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

TEST_F(BranchesTests, changeBranchRef_fullName)
{
    const auto& commits = repository->Commits();
    const auto& branches = repository->Branches();
    auto secondCommitHash = commits.createCommit("second commit");

    auto hashBeforeChange = branches.getHashBranchRefersTo("main");

    ASSERT_EQ(hashBeforeChange, secondCommitHash);

    branches.changeBranchRef("refs/heads/main", initialCommitHash);
    auto hashAfterChange = branches.getHashBranchRefersTo("main");

    EXPECT_EQ(hashAfterChange, initialCommitHash);
}

TEST_F(BranchesTests, changeBranchRef_shortName)
{
    const auto& commits = repository->Commits();
    const auto& branches = repository->Branches();
    auto secondCommitHash = commits.createCommit("second commit");

    auto hashBeforeChange = branches.getHashBranchRefersTo("main");

    ASSERT_EQ(hashBeforeChange, secondCommitHash);

    branches.changeBranchRef("main", initialCommitHash);
    auto hashAfterChange = branches.getHashBranchRefersTo("main");

    EXPECT_EQ(hashAfterChange, initialCommitHash);
}
