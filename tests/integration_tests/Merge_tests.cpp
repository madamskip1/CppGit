#include "BaseRepositoryFixture.hpp"
#include "Branches.hpp"
#include "Commits.hpp"
#include "Merge.hpp"
#include "_details/FileUtility.hpp"

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
    auto merge = repository->Merge();

    commits.createCommit("Initial commit");

    EXPECT_TRUE(merge.canFastForward("main"));
}

TEST_F(MergeTests, canFastForward_head)
{
    auto commits = repository->Commits();
    auto merge = repository->Merge();

    commits.createCommit("Initial commit");

    EXPECT_TRUE(merge.canFastForward("HEAD"));
}

TEST_F(MergeTests, canFastForward_bothBranchesSameCommit)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");


    EXPECT_TRUE(merge.canFastForward("second_branch"));
}

TEST_F(MergeTests, canFastForward_linearBehind)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");


    EXPECT_TRUE(merge.canFastForward("main"));
}

TEST_F(MergeTests, canFastForward_linearAhead)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");


    EXPECT_FALSE(merge.canFastForward("second_branch"));
}

TEST_F(MergeTests, canFastForward_changesInBothBranches)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    commits.createCommit("Third commit");


    EXPECT_FALSE(merge.canFastForward("main"));
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
    auto merge = repository->Merge();

    commits.createCommit("Initial commit");

    EXPECT_FALSE(merge.isAnythingToMerge("main"));
}

TEST_F(MergeTests, anythingToMerge_head)
{
    auto commits = repository->Commits();
    auto merge = repository->Merge();

    commits.createCommit("Initial commit");

    EXPECT_FALSE(merge.isAnythingToMerge("HEAD"));
}

TEST_F(MergeTests, anythingToMerge_bothBranchesSameCommit)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");


    EXPECT_FALSE(merge.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_linearBehind)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");


    EXPECT_TRUE(merge.isAnythingToMerge("main"));
}

TEST_F(MergeTests, anythingToMerge_linearAhead)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");


    EXPECT_FALSE(merge.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_changesInBothBranches)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");
    commits.createCommit("Third commit");


    EXPECT_TRUE(merge.isAnythingToMerge("main"));
}

TEST_F(MergeTests, mergeFastForward_emptyRepo_sameBranch)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeFastForward("main"), std::runtime_error);
}

TEST_F(MergeTests, mergeFastForward_sameBranch)
{
    auto commits = repository->Commits();
    auto merge = repository->Merge();


    auto initialCommitHash = commits.createCommit("Initial commit");
    auto mergeCommitHash = merge.mergeFastForward("main");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), initialCommitHash);
    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
}

TEST_F(MergeTests, mergeFastForward_bothBranchesSameCommit)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    auto mergeCommitHash = merge.mergeFastForward("second_branch");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), initialCommitHash);
    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
}

TEST_F(MergeTests, mergeFastForward_linearBehind)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();
    auto index = repository->Index();

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "initial");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "second");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");

    auto mergeCommitHash = merge.mergeFastForward("main");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), secondCommitHash);
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "second");
}

TEST_F(MergeTests, mergeFastForward_linearAhead)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    auto secondCommitHash = commits.createCommit("Second commit");
    auto mergeCommitHash = merge.mergeFastForward("second_branch");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash, secondCommitHash);
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
}

TEST_F(MergeTests, mergeFastForward_changesInBothBranches)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");
    commits.createCommit("Third commit");

    auto mergeFFResult = merge.mergeFastForward("main");


    ASSERT_FALSE(mergeFFResult.has_value());
    EXPECT_EQ(mergeFFResult.error(), CppGit::Error::MERGE_FF_BRANCHES_DIVERGENCE);
}

TEST_F(MergeTests, mergeFastForward_dirtyRepo)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    auto mergeFFResult = merge.mergeFastForward("main");


    ASSERT_FALSE(mergeFFResult.has_value());
    EXPECT_EQ(mergeFFResult.error(), CppGit::Error::DIRTY_WORKTREE);
}

TEST_F(MergeTests, mergeFastForward_untrackedFile)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    auto secondCommitHash = commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    auto mergeCommitHash = merge.mergeFastForward("main");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), secondCommitHash);
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
}

TEST_F(MergeTests, mergeNoFastForward_emptyRepo_sameBranch)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeNoFastForward("main", "Merge commit"), std::runtime_error);
}

TEST_F(MergeTests, mergeNoFastForward_sameBranch)
{
    auto commits = repository->Commits();
    auto merge = repository->Merge();


    auto initialCommitHash = commits.createCommit("Initial commit");
    auto mergeNoFFResult = merge.mergeNoFastForward("main", "merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::Error::MERGE_NOTHING_TO_MERGE);
}

TEST_F(MergeTests, mergeNoFastForward_bothBranchesSameCommit)
{
    auto commits = repository->Commits();
    auto merge = repository->Merge();
    auto branches = repository->Branches();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");

    auto mergeNoFFResult = merge.mergeNoFastForward("second_branch", "merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::Error::MERGE_NOTHING_TO_MERGE);
}

TEST_F(MergeTests, mergeNoFastForward_linearBehind)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto index = repository->Index();
    auto merge = repository->Merge();


    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    auto mergeCommitHash = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), commits.getHeadCommitHash());
    auto mergeCommitInfo = commits.getCommitInfo(mergeCommitHash.value());
    EXPECT_EQ(mergeCommitInfo.getMessage(), "Merge commit");
    ASSERT_EQ(mergeCommitInfo.getParents().size(), 2);
    EXPECT_EQ(mergeCommitInfo.getParents()[0], initialCommitHash);
    EXPECT_EQ(mergeCommitInfo.getParents()[1], secondCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));
}

TEST_F(MergeTests, mergeNoFastForward_linearAhead)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto index = repository->Index();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Second commit");

    auto mergeNoFFResult = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::Error::MERGE_NOTHING_TO_MERGE);
}

TEST_F(MergeTests, mergeNoFastForward_dirtyRepo)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto index = repository->Index();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    auto mergeNoFFResult = merge.mergeNoFastForward("second_branch", "merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::Error::DIRTY_WORKTREE);
}

TEST_F(MergeTests, mergeNoFastForward_changesInBothBranches_noConflict)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto index = repository->Index();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto mergeCommitHash = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), commits.getHeadCommitHash());
    auto mergeCommitInfo = commits.getCommitInfo(mergeCommitHash.value());
    EXPECT_EQ(mergeCommitInfo.getMessage(), "Merge commit");
    ASSERT_EQ(mergeCommitInfo.getParents().size(), 2);
    EXPECT_EQ(mergeCommitInfo.getParents()[0], thirdCommitHash);
    EXPECT_EQ(mergeCommitInfo.getParents()[1], secondCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));
}

TEST_F(MergeTests, mergeNoFastForward_conflict_changesInSameFile)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");

    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    commits.createCommit("Third commit");

    auto mergeNoFFResult = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::Error::MERGE_NO_FF_CONFLICT);
    EXPECT_TRUE(merge.isMergeInProgress());
    EXPECT_TRUE(merge.isThereAnyConflict());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified 1.\n>>>>>>> main\n");
}

TEST_F(MergeTests, mergeNoFastForward_conflict_bothConflictAndNotFiles)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");

    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!");
    index.add("file.txt");
    index.add("file2.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2!");
    index.add("file.txt");
    commits.createCommit("Third commit");

    auto mergeNoFFResult = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::Error::MERGE_NO_FF_CONFLICT);
    EXPECT_TRUE(merge.isMergeInProgress());
    EXPECT_TRUE(merge.isThereAnyConflict());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2!\n=======\nHello, World! Modified 1!\n>>>>>>> main\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World 2!");
}

TEST_F(MergeTests, mergeNoFastForward_conflictTwoFiles)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!.");
    index.add("file.txt");
    index.add("file2.txt");
    commits.createCommit("Initial commit");

    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2! Modified 1!");
    index.add("file.txt");
    index.add("file2.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2! Modified 2!");
    index.add("file.txt");
    index.add("file2.txt");
    commits.createCommit("Third commit");

    auto mergeNoFFResult = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::Error::MERGE_NO_FF_CONFLICT);
    EXPECT_TRUE(merge.isMergeInProgress());
    EXPECT_TRUE(merge.isThereAnyConflict());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2!\n=======\nHello, World! Modified 1!\n>>>>>>> main\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "<<<<<<< HEAD\nHello, World 2! Modified 2!\n=======\nHello, World 2! Modified 1!\n>>>>>>> main\n");
}

TEST_F(MergeTests, mergeNoFastForward_resolveConflict)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");

    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    auto thirdCommithash = commits.createCommit("Third commit");
    merge.mergeNoFastForward("main", "Merge commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Merge resolved.");
    index.add("file.txt");

    auto mergeCommitHash = merge.continueMerge();


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_FALSE(merge.isMergeInProgress());
    EXPECT_FALSE(merge.isThereAnyConflict());
    EXPECT_EQ(mergeCommitHash.value(), commits.getHeadCommitHash());
    auto mergeCommitInfo = commits.getCommitInfo(mergeCommitHash.value());
    EXPECT_EQ(mergeCommitInfo.getMessage(), "Merge commit");
    ASSERT_EQ(mergeCommitInfo.getParents().size(), 2);
    EXPECT_EQ(mergeCommitInfo.getParents()[0], thirdCommithash);
    EXPECT_EQ(mergeCommitInfo.getParents()[1], secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Merge resolved.");
}


TEST_F(MergeTests, mergeNoFastForward_abort)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");

    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    commits.createCommit("Third commit");

    merge.mergeNoFastForward("main", "Merge commit");

    auto abortResult = merge.abortMerge();


    EXPECT_EQ(abortResult, CppGit::Error::NO_ERROR);
    EXPECT_FALSE(merge.isMergeInProgress());
    EXPECT_FALSE(index.isDirty());
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MSG"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MODE"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Modified 2.");
}

TEST_F(MergeTests, mergeNoFastForward_conflict_fileNotExistsInAncestor)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    commits.createCommit("Third commit");

    auto mergeNoFFResult = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::Error::MERGE_NO_FF_CONFLICT);
    EXPECT_TRUE(merge.isThereAnyConflict());
    EXPECT_TRUE(merge.isMergeInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified 1.\n>>>>>>> main\n");
}

TEST_F(MergeTests, mergeNoFastForward_abort_noMergeInProgress)
{
    auto commits = repository->Commits();
    auto merge = repository->Merge();

    commits.createCommit("Initial commit");
    auto abortResult = merge.abortMerge();

    EXPECT_EQ(abortResult, CppGit::Error::NO_MERGE_IN_PROGRESS);
}

TEST_F(MergeTests, mergeNoFastForward_continue_noMergeInProgress)
{
    auto commits = repository->Commits();
    auto merge = repository->Merge();

    commits.createCommit("Initial commit");
    auto continueResult = merge.continueMerge();

    ASSERT_FALSE(continueResult.has_value());
    EXPECT_EQ(continueResult.error(), CppGit::Error::NO_MERGE_IN_PROGRESS);
}

TEST_F(MergeTests, mergeNoFastForward_continue_conflict)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto merge = repository->Merge();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");

    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    commits.createCommit("Third commit");

    merge.mergeNoFastForward("main", "Merge commit");

    auto continueResult = merge.continueMerge();

    ASSERT_FALSE(continueResult.has_value());
    EXPECT_EQ(continueResult.error(), CppGit::Error::MERGE_NO_FF_CONFLICT);
}
