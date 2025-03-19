#include "BaseRepositoryFixture.hpp"

#include <CppGit/BranchesManager.hpp>
#include <CppGit/CommitsManager.hpp>
#include <CppGit/Merger.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <gtest/gtest.h>

class MergeTests : public BaseRepositoryFixture
{
};

TEST_F(MergeTests, canFastForward_sameBranch)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();

    commitsManager.createCommit("Initial commit");

    EXPECT_TRUE(merger.canFastForward("main"));
}

TEST_F(MergeTests, canFastForward_head)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();

    commitsManager.createCommit("Initial commit");

    EXPECT_TRUE(merger.canFastForward("HEAD"));
}

TEST_F(MergeTests, canFastForward_bothBranchesSameCommit)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");


    EXPECT_TRUE(merger.canFastForward("second_branch"));
}

TEST_F(MergeTests, canFastForward_linearBehind)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    commitsManager.createCommit("Second commit");
    branchesManager.changeBranch("second_branch");


    EXPECT_TRUE(merger.canFastForward("main"));
}

TEST_F(MergeTests, canFastForward_linearAhead)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    commitsManager.createCommit("Second commit");


    EXPECT_FALSE(merger.canFastForward("second_branch"));
}

TEST_F(MergeTests, canFastForward_changesInBothBranches)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");
    commitsManager.createCommit("Third commit");


    EXPECT_FALSE(merger.canFastForward("main"));
}

TEST_F(MergeTests, anythingToMerge_sameBranch)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();

    commitsManager.createCommit("Initial commit");

    EXPECT_FALSE(merger.isAnythingToMerge("main"));
}

TEST_F(MergeTests, anythingToMerge_head)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();

    commitsManager.createCommit("Initial commit");

    EXPECT_FALSE(merger.isAnythingToMerge("HEAD"));
}

TEST_F(MergeTests, anythingToMerge_bothBranchesSameCommit)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");


    EXPECT_FALSE(merger.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_linearBehind)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    commitsManager.createCommit("Second commit");
    branchesManager.changeBranch("second_branch");


    EXPECT_TRUE(merger.isAnythingToMerge("main"));
}

TEST_F(MergeTests, anythingToMerge_linearAhead)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    commitsManager.createCommit("Second commit");


    EXPECT_FALSE(merger.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_changesInBothBranches)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    commitsManager.createCommit("Second commit");
    branchesManager.changeBranch("second_branch");
    commitsManager.createCommit("Third commit");


    EXPECT_TRUE(merger.isAnythingToMerge("main"));
}

TEST_F(MergeTests, mergeFastForward_sameBranch)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    const auto mergeCommitHash = merger.mergeFastForward("main");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), initialCommitHash);
    EXPECT_EQ(commitsManager.getHeadCommitHash(), initialCommitHash);
}

TEST_F(MergeTests, mergeFastForward_bothBranchesSameCommit)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    const auto mergeCommitHash = merger.mergeFastForward("second_branch");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), initialCommitHash);
    EXPECT_EQ(commitsManager.getHeadCommitHash(), initialCommitHash);
}

TEST_F(MergeTests, mergeFastForward_linearBehind)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "initial");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "second");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    branchesManager.changeBranch("second_branch");

    const auto mergeCommitHash = merger.mergeFastForward("main");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), secondCommitHash);
    EXPECT_EQ(commitsManager.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "second");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
}

TEST_F(MergeTests, mergeFastForward_linearAhead)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    const auto mergeCommitHash = merger.mergeFastForward("second_branch");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash, secondCommitHash);
    EXPECT_EQ(commitsManager.getHeadCommitHash(), secondCommitHash);
}

TEST_F(MergeTests, mergeFastForward_changesInBothBranches)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    commitsManager.createCommit("Second commit");
    branchesManager.changeBranch("second_branch");
    commitsManager.createCommit("Third commit");

    const auto mergeFFResult = merger.mergeFastForward("main");


    ASSERT_FALSE(mergeFFResult.has_value());
    EXPECT_EQ(mergeFFResult.error(), CppGit::MergeResult::FF_BRANCHES_DIVERGENCE);
}

TEST_F(MergeTests, mergeFastForward_untrackedFile)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    branchesManager.changeBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    const auto mergeCommitHash = merger.mergeFastForward("main");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), secondCommitHash);
    EXPECT_EQ(commitsManager.getHeadCommitHash(), secondCommitHash);
}

TEST_F(MergeTests, mergeNoFastForward_sameBranch)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    const auto mergeNoFFResult = merger.mergeNoFastForward("main", "merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NOTHING_TO_MERGE);
}

TEST_F(MergeTests, mergeNoFastForward_bothBranchesSameCommit)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");

    const auto mergeNoFFResult = merger.mergeNoFastForward("second_branch", "merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NOTHING_TO_MERGE);
}

TEST_F(MergeTests, mergeNoFastForward_linearBehind)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");

    const auto mergeCommitHash = merger.mergeNoFastForward("main", "Merge commit");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), commitsManager.getHeadCommitHash());
    const auto mergeCommitInfo = commitsManager.getCommitInfo(mergeCommitHash.value());
    EXPECT_EQ(mergeCommitInfo.getMessage(), "Merge commit");
    ASSERT_EQ(mergeCommitInfo.getParents().size(), 2);
    EXPECT_EQ(mergeCommitInfo.getParents()[0], initialCommitHash);
    EXPECT_EQ(mergeCommitInfo.getParents()[1], secondCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
}

TEST_F(MergeTests, mergeNoFastForward_linearAhead)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    branchesManager.changeBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");

    const auto mergeNoFFResult = merger.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NOTHING_TO_MERGE);
}

TEST_F(MergeTests, mergeNoFastForward_changesInBothBranches_noConflict)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    const auto mergeCommitHash = merger.mergeNoFastForward("main", "Merge commit");


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_EQ(mergeCommitHash.value(), commitsManager.getHeadCommitHash());
    const auto mergeCommitInfo = commitsManager.getCommitInfo(mergeCommitHash.value());
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
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");

    branchesManager.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    indexManager.add("file.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    const auto mergeNoFFResult = merger.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NO_FF_CONFLICT);
    EXPECT_TRUE(merger.isMergeInProgress());
    EXPECT_TRUE(merger.isThereAnyConflict());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified 1.\n>>>>>>> main\n");
}

TEST_F(MergeTests, mergeNoFastForward_conflict_bothConflictAndNotFiles)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");

    branchesManager.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!");
    indexManager.add("file.txt");
    indexManager.add("file2.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2!");
    indexManager.add("file.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    const auto mergeNoFFResult = merger.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NO_FF_CONFLICT);
    EXPECT_TRUE(merger.isMergeInProgress());
    EXPECT_TRUE(merger.isThereAnyConflict());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2!\n=======\nHello, World! Modified 1!\n>>>>>>> main\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World 2!");
}

TEST_F(MergeTests, mergeNoFastForward_conflictTwoFiles)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!.");
    indexManager.add("file.txt");
    indexManager.add("file2.txt");
    commitsManager.createCommit("Initial commit");

    branchesManager.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2! Modified 1!");
    indexManager.add("file.txt");
    indexManager.add("file2.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2! Modified 2!");
    indexManager.add("file.txt");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    const auto mergeNoFFResult = merger.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NO_FF_CONFLICT);
    EXPECT_TRUE(merger.isMergeInProgress());
    EXPECT_TRUE(merger.isThereAnyConflict());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2!\n=======\nHello, World! Modified 1!\n>>>>>>> main\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "<<<<<<< HEAD\nHello, World 2! Modified 2!\n=======\nHello, World 2! Modified 1!\n>>>>>>> main\n");
}

TEST_F(MergeTests, mergeNoFastForward_resolveConflict)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");

    branchesManager.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    indexManager.add("file.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");
    merger.mergeNoFastForward("main", "Merge commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Merge resolved.");
    indexManager.add("file.txt");

    const auto mergeCommitHash = merger.continueMerge();


    ASSERT_TRUE(mergeCommitHash.has_value());
    EXPECT_FALSE(merger.isMergeInProgress());
    EXPECT_FALSE(merger.isThereAnyConflict());
    EXPECT_EQ(mergeCommitHash.value(), commitsManager.getHeadCommitHash());
    const auto mergeCommitInfo = commitsManager.getCommitInfo(mergeCommitHash.value());
    EXPECT_EQ(mergeCommitInfo.getMessage(), "Merge commit");
    ASSERT_EQ(mergeCommitInfo.getParents().size(), 2);
    EXPECT_EQ(mergeCommitInfo.getParents()[0], thirdCommitHash);
    EXPECT_EQ(mergeCommitInfo.getParents()[1], secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Merge resolved.");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}


TEST_F(MergeTests, mergeNoFastForward_abort)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");

    branchesManager.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    indexManager.add("file.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    merger.mergeNoFastForward("main", "Merge commit");

    merger.abortMerge();


    EXPECT_FALSE(merger.isMergeInProgress());
    EXPECT_FALSE(indexManager.isDirty());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MSG"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MODE"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Modified 2.");
}

TEST_F(MergeTests, mergeNoFastForward_conflict_fileNotExistsInAncestor)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    indexManager.add("file.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    const auto mergeNoFFResult = merger.mergeNoFastForward("main", "Merge commit");


    ASSERT_FALSE(mergeNoFFResult.has_value());
    EXPECT_EQ(mergeNoFFResult.error(), CppGit::MergeResult::NO_FF_CONFLICT);
    EXPECT_TRUE(merger.isThereAnyConflict());
    EXPECT_TRUE(merger.isMergeInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified 1.\n>>>>>>> main\n");
}

TEST_F(MergeTests, mergeNoFastForward_continue_conflict)
{
    const auto merger = repository->Merger();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");

    branchesManager.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 1.");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    indexManager.add("file.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    merger.mergeNoFastForward("main", "Merge commit");

    const auto continueResult = merger.continueMerge();

    ASSERT_FALSE(continueResult.has_value());
    EXPECT_EQ(continueResult.error(), CppGit::MergeResult::NO_FF_CONFLICT);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}
