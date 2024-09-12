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

    const auto currentBranch = branches.getCurrentBranchRef();
    EXPECT_EQ(currentBranch, "refs/heads/main");

    const auto hashBranchRefersTo = branches.getHashBranchRefersTo("new_branch");
    EXPECT_EQ(hashBranchRefersTo, initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromBranch_fullName)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    branches.createBranch("new_branch");

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");

    auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranchFromBranch("refs/heads/new_branch_from_new", allBranches[1]);

    allBranches = branches.getAllBranches();
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

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");

    auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranchFromBranch("new_branch_from_new", allBranches[1]);

    allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 3);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(allBranches[2].getRefName(), "refs/heads/new_branch_from_new");

    EXPECT_EQ(branches.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch"), initialCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch_from_new"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromCommit_fullName)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();

    auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranchFromCommit("refs/heads/new_branch_from_commit", initialCommitHash);

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[2].getRefName(), "refs/heads/new_branch_from_commit");

    EXPECT_EQ(branches.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromCommit_shortName)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();

    auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranchFromCommit("refs/heads/new_branch_from_commit", initialCommitHash);

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[2].getRefName(), "new_branch_from_commit");

    EXPECT_EQ(branches.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch"), initialCommitHash);
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

TEST_F(BranchesTests, currentBranchInfo)
{
    const auto& branches = repository->Branches();
    const auto currentBranch = branches.getCurrentBranch();

    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    EXPECT_EQ(currentBranch.getUpstreamPull(), "");
    EXPECT_EQ(currentBranch.getUpstreamPush(), "");
    EXPECT_TRUE(currentBranch.isLocalBranch());
}

TEST_F(BranchesTests, changeBranch_shortName)
{
    const auto& branches = repository->Branches();
    branches.createBranch("new_branch");

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(branches.getCurrentBranchRef(), "refs/heads/main");

    branches.changeCurrentBranch("new_branch");

    EXPECT_EQ(branches.getCurrentBranchRef(), "refs/heads/new_branch");
}

TEST_F(BranchesTests, changeBranch_fullName)
{
    const auto& branches = repository->Branches();
    branches.createBranch("new_branch");

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(branches.getCurrentBranchRef(), "refs/heads/main");

    branches.changeCurrentBranch("refs/heads/new_branch");

    EXPECT_EQ(branches.getCurrentBranchRef(), "refs/heads/new_branch");
}
