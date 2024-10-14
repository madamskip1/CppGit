#include "BaseRepositoryFixture.hpp"
#include "Branches.hpp"
#include "Commits.hpp"
#include "Merge.hpp"

#include <fstream>
#include <gtest/gtest.h>

class MergeTests : public BaseRepositoryFixture
{
};

TEST_F(MergeTests, canFastForward_emptyRepo_sameBranch)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.canFastForward("main"), std::runtime_error);
}

TEST_F(MergeTests, canFastForward_emptyRepo_head)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.canFastForward("HEAD"), std::runtime_error);
}

TEST_F(MergeTests, canFastForward_sameBranch)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto merge = repository->Merge();

    EXPECT_TRUE(merge.canFastForward("main"));
}

TEST_F(MergeTests, canFastForward_head)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto merge = repository->Merge();

    EXPECT_TRUE(merge.canFastForward("HEAD"));
}

TEST_F(MergeTests, canFastForward_bothBranchesSameCommit)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    auto merge = repository->Merge();

    EXPECT_TRUE(merge.canFastForward("second_branch"));
}

TEST_F(MergeTests, canFastForward_linearBehind)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");
    branches.changeCurrentBranch("second_branch");

    commits.createCommit("Second commit");

    branches.changeCurrentBranch("main");

    auto merge = repository->Merge();

    EXPECT_TRUE(merge.canFastForward("second_branch"));
}

TEST_F(MergeTests, canFastForward_linearAhead)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    commits.createCommit("Second commit");

    auto merge = repository->Merge();

    EXPECT_FALSE(merge.canFastForward("second_branch"));
}

TEST_F(MergeTests, canFastForward_changesInBothBranches)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    commits.createCommit("Third commit");

    branches.changeCurrentBranch("main");

    auto merge = repository->Merge();

    EXPECT_FALSE(merge.canFastForward("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_emptyRepo_sameBranch)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.isAnythingToMerge("main"), std::runtime_error);
}

TEST_F(MergeTests, anythingToMerge_emptyRepo_head)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.isAnythingToMerge("HEAD"), std::runtime_error);
}

TEST_F(MergeTests, anythingToMerge_sameBranch)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto merge = repository->Merge();

    EXPECT_FALSE(merge.isAnythingToMerge("main"));
}

TEST_F(MergeTests, anythingToMerge_head)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto merge = repository->Merge();

    EXPECT_FALSE(merge.isAnythingToMerge("HEAD"));
}

TEST_F(MergeTests, anythingToMerge_bothBranchesSameCommit)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    auto merge = repository->Merge();

    EXPECT_FALSE(merge.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_linearBehind)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");
    branches.changeCurrentBranch("second_branch");

    commits.createCommit("Second commit");

    branches.changeCurrentBranch("main");

    auto merge = repository->Merge();

    EXPECT_TRUE(merge.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_linearAhead)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    commits.createCommit("Second commit");

    auto merge = repository->Merge();

    EXPECT_FALSE(merge.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_changesInBothBranches)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    commits.createCommit("Third commit");

    branches.changeCurrentBranch("main");

    auto merge = repository->Merge();

    EXPECT_TRUE(merge.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, mergeFastForward_emptyRepo_sameBranch)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeFastForward("main"), std::runtime_error);
}

TEST_F(MergeTests, mergeFastForward_sameBranch)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto merge = repository->Merge();
    auto mergeCommitHash = merge.mergeFastForward("main");

    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(mergeCommitHash, initialCommitHash);
}

TEST_F(MergeTests, mergeFastForward_bothBranchesSameCommit)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    auto merge = repository->Merge();
    auto mergeCommitHash = merge.mergeFastForward("second_branch");

    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(mergeCommitHash, initialCommitHash);
}

TEST_F(MergeTests, mergeFastForward_linearBehind)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");
    branches.changeCurrentBranch("second_branch");

    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("main");

    auto merge = repository->Merge();
    auto mergeCommitHash = merge.mergeFastForward("second_branch");

    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(mergeCommitHash, secondCommitHash);
}

TEST_F(MergeTests, mergeFastForward_linearAhead)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    auto secondCommitHash = commits.createCommit("Second commit");

    auto merge = repository->Merge();
    auto mergeCommitHash = merge.mergeFastForward("second_branch");

    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(mergeCommitHash, secondCommitHash);
}

TEST_F(MergeTests, mergeFastForward_changesInBothBranches)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    auto thirdCommitHash = commits.createCommit("Third commit");

    branches.changeCurrentBranch("main");

    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeFastForward("second_branch"), std::runtime_error);
}

TEST_F(MergeTests, mergeFastForward_dirtyRepo)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto initialCommitHash = commits.createCommit("Initial commit");

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Second commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");
    branches.changeCurrentBranch("second_branch");

    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("main");

    file.open(repositoryPath / "file.txt", std::ios::app);
    file << "Hello, World! Modified.";
    file.close();

    ASSERT_TRUE(index.isDirty());

    auto merge = repository->Merge();
    EXPECT_THROW(merge.mergeFastForward("second_branch"), std::runtime_error);
}
