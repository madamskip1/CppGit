#include "BaseRepositoryFixture.hpp"

#include <CppGit/Branches.hpp>
#include <CppGit/Commits.hpp>
#include <CppGit/Merge.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <gtest/gtest.h>

class MergeTests : public BaseRepositoryFixture
{
};

TEST_F(MergeTests, canFastForward_sameBranch)
{
    const auto commits = repository->Commits();
    const auto merge = repository->Merge();

    commits.createCommit("Initial commit");

    EXPECT_TRUE(merge.canFastForward("main"));
}

TEST_F(MergeTests, canFastForward_head)
{
    const auto commits = repository->Commits();
    const auto merge = repository->Merge();

    commits.createCommit("Initial commit");

    EXPECT_TRUE(merge.canFastForward("HEAD"));
}

TEST_F(MergeTests, canFastForward_bothBranchesSameCommit)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");


    EXPECT_TRUE(merge.canFastForward("second_branch"));
}

TEST_F(MergeTests, canFastForward_linearBehind)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");


    EXPECT_TRUE(merge.canFastForward("main"));
}

TEST_F(MergeTests, canFastForward_linearAhead)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");


    EXPECT_FALSE(merge.canFastForward("second_branch"));
}

TEST_F(MergeTests, canFastForward_changesInBothBranches)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    commits.createCommit("Third commit");


    EXPECT_FALSE(merge.canFastForward("main"));
}

TEST_F(MergeTests, anythingToMerge_sameBranch)
{
    const auto commits = repository->Commits();
    const auto merge = repository->Merge();

    commits.createCommit("Initial commit");

    EXPECT_FALSE(merge.isAnythingToMerge("main"));
}

TEST_F(MergeTests, anythingToMerge_head)
{
    const auto commits = repository->Commits();
    const auto merge = repository->Merge();

    commits.createCommit("Initial commit");

    EXPECT_FALSE(merge.isAnythingToMerge("HEAD"));
}

TEST_F(MergeTests, anythingToMerge_bothBranchesSameCommit)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");


    EXPECT_FALSE(merge.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_linearBehind)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");


    EXPECT_TRUE(merge.isAnythingToMerge("main"));
}

TEST_F(MergeTests, anythingToMerge_linearAhead)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");


    EXPECT_FALSE(merge.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_changesInBothBranches)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");
    commits.createCommit("Third commit");


    EXPECT_TRUE(merge.isAnythingToMerge("main"));
}

TEST_F(MergeTests, mergeFastForward_sameBranch)
{
    const auto commits = repository->Commits();
    const auto merge = repository->Merge();


    const auto initialCommitHash = commits.createCommit("Initial commit");
    const auto mergeCommitHash = merge.mergeFastForward("main");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), initialCommitHash);
    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
}

TEST_F(MergeTests, mergeFastForward_bothBranchesSameCommit)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    const auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    const auto mergeCommitHash = merge.mergeFastForward("second_branch");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), initialCommitHash);
    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
}

TEST_F(MergeTests, mergeFastForward_linearBehind)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();
    const auto index = repository->Index();

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "initial");
    index.add("file.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "second");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");

    const auto mergeCommitHash = merge.mergeFastForward("main");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), secondCommitHash);
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "second");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
}

TEST_F(MergeTests, mergeFastForward_linearAhead)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    const auto secondCommitHash = commits.createCommit("Second commit");
    const auto mergeCommitHash = merge.mergeFastForward("second_branch");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash, secondCommitHash);
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
}

TEST_F(MergeTests, mergeFastForward_changesInBothBranches)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");
    commits.createCommit("Third commit");

    const auto mergeFFResult = merge.mergeFastForward("main");


    ASSERT_FALSE(mergeFFResult.has_value());
    EXPECT_EQ(mergeFFResult.error(), CppGit::MergeResult::FF_BRANCHES_DIVERGENCE);
}

TEST_F(MergeTests, mergeFastForward_untrackedFile)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    const auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    const auto secondCommitHash = commits.createCommit("Second commit");
    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    const auto mergeCommitHash = merge.mergeFastForward("main");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), secondCommitHash);
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
}

TEST_F(MergeTests, mergeNoFastForward_sameBranch)
{
    const auto commits = repository->Commits();
    const auto merge = repository->Merge();


    const auto initialCommitHash = commits.createCommit("Initial commit");
    const auto mergeNoFFResult = merge.mergeNoFastForward("main", "merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NOTHING_TO_MERGE);
}

TEST_F(MergeTests, mergeNoFastForward_bothBranchesSameCommit)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");

    const auto mergeNoFFResult = merge.mergeNoFastForward("second_branch", "merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NOTHING_TO_MERGE);
}

TEST_F(MergeTests, mergeNoFastForward_linearBehind)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto index = repository->Index();
    const auto merge = repository->Merge();


    const auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    const auto mergeCommitHash = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), commits.getHeadCommitHash());
    const auto mergeCommitInfo = commits.getCommitInfo(mergeCommitHash.value());
    EXPECT_EQ(mergeCommitInfo.getMessage(), "Merge commit");
    ASSERT_EQ(mergeCommitInfo.getParents().size(), 2);
    EXPECT_EQ(mergeCommitInfo.getParents()[0], initialCommitHash);
    EXPECT_EQ(mergeCommitInfo.getParents()[1], secondCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
}

TEST_F(MergeTests, mergeNoFastForward_linearAhead)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto index = repository->Index();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Second commit");

    const auto mergeNoFFResult = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NOTHING_TO_MERGE);
}

TEST_F(MergeTests, mergeNoFastForward_changesInBothBranches_noConflict)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto index = repository->Index();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");

    const auto mergeCommitHash = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), commits.getHeadCommitHash());
    const auto mergeCommitInfo = commits.getCommitInfo(mergeCommitHash.value());
    EXPECT_EQ(mergeCommitInfo.getMessage(), "Merge commit");
    ASSERT_EQ(mergeCommitInfo.getParents().size(), 2);
    EXPECT_EQ(mergeCommitInfo.getParents()[0], thirdCommitHash);
    EXPECT_EQ(mergeCommitInfo.getParents()[1], secondCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(MergeTests, mergeNoFastForward_conflict_changesInSameFile)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");

    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");

    const auto mergeNoFFResult = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NO_FF_CONFLICT);
    EXPECT_TRUE(merge.isMergeInProgress());
    EXPECT_TRUE(merge.isThereAnyConflict());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified 1.\n>>>>>>> main\n");
}

TEST_F(MergeTests, mergeNoFastForward_conflict_bothConflictAndNotFiles)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");

    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!");
    index.add("file.txt");
    index.add("file2.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2!");
    index.add("file.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");

    const auto mergeNoFFResult = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NO_FF_CONFLICT);
    EXPECT_TRUE(merge.isMergeInProgress());
    EXPECT_TRUE(merge.isThereAnyConflict());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2!\n=======\nHello, World! Modified 1!\n>>>>>>> main\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World 2!");
}

TEST_F(MergeTests, mergeNoFastForward_conflictTwoFiles)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


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
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2! Modified 2!");
    index.add("file.txt");
    index.add("file2.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");

    const auto mergeNoFFResult = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NO_FF_CONFLICT);
    EXPECT_TRUE(merge.isMergeInProgress());
    EXPECT_TRUE(merge.isThereAnyConflict());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2!\n=======\nHello, World! Modified 1!\n>>>>>>> main\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "<<<<<<< HEAD\nHello, World 2! Modified 2!\n=======\nHello, World 2! Modified 1!\n>>>>>>> main\n");
}

TEST_F(MergeTests, mergeNoFastForward_resolveConflict)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");

    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");
    merge.mergeNoFastForward("main", "Merge commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Merge resolved.");
    index.add("file.txt");

    const auto mergeCommitHash = merge.continueMerge();


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_FALSE(merge.isMergeInProgress());
    EXPECT_FALSE(merge.isThereAnyConflict());
    EXPECT_EQ(mergeCommitHash.value(), commits.getHeadCommitHash());
    const auto mergeCommitInfo = commits.getCommitInfo(mergeCommitHash.value());
    EXPECT_EQ(mergeCommitInfo.getMessage(), "Merge commit");
    ASSERT_EQ(mergeCommitInfo.getParents().size(), 2);
    EXPECT_EQ(mergeCommitInfo.getParents()[0], thirdCommitHash);
    EXPECT_EQ(mergeCommitInfo.getParents()[1], secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Merge resolved.");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}


TEST_F(MergeTests, mergeNoFastForward_abort)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");

    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");

    merge.mergeNoFastForward("main", "Merge commit");

    merge.abortMerge();


    EXPECT_FALSE(merge.isMergeInProgress());
    EXPECT_FALSE(index.isDirty());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MSG"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MODE"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Modified 2.");
}

TEST_F(MergeTests, mergeNoFastForward_conflict_fileNotExistsInAncestor)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");

    const auto mergeNoFFResult = merge.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NO_FF_CONFLICT);
    EXPECT_TRUE(merge.isThereAnyConflict());
    EXPECT_TRUE(merge.isMergeInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified 1.\n>>>>>>> main\n");
}

TEST_F(MergeTests, mergeNoFastForward_continue_conflict)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto branches = repository->Branches();
    const auto merge = repository->Merge();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");

    branches.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");

    merge.mergeNoFastForward("main", "Merge commit");

    const auto continueResult = merge.continueMerge();

    ASSERT_FALSE(continueResult.has_value());
    EXPECT_EQ(continueResult.error(), CppGit::MergeResult::NO_FF_CONFLICT);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}
