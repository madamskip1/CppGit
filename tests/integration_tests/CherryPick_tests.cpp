#include "BaseRepositoryFixture.hpp"

#include <CppGit/BranchesManager.hpp>
#include <CppGit/CherryPicker.hpp>
#include <CppGit/Commit.hpp>
#include <CppGit/CommitsManager.hpp>
#include <CppGit/IndexManager.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <filesystem>
#include <gtest/gtest.h>

class CherryPickTests : public BaseRepositoryFixture
{
};

TEST_F(CherryPickTests, simpleCherryPick)
{
    const auto cherryPicker = repository->CherryPicker();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second-branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    branchesManager.changeBranch("second-branch");

    const auto cherryPickedHash = cherryPicker.cherryPick(secondCommitHash);


    ASSERT_TRUE(cherryPickedHash.has_value());
    const auto cherryPickedInfo = commitsManager.getCommitInfo(cherryPickedHash.value());
    EXPECT_EQ(cherryPickedHash, commitsManager.getHeadCommitHash());
    EXPECT_NE(cherryPickedHash, secondCommitHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_keep)
{
    const auto cherryPicker = repository->CherryPicker();
    const auto commitsManager = repository->CommitsManager();


    const auto initialCommitHash = createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    const auto cherryPickedHash = cherryPicker.cherryPick(initialCommitHash, CppGit::CherryPickEmptyCommitStrategy::KEEP);


    ASSERT_TRUE(cherryPickedHash.has_value());
    const auto cherryPickedInfo = commitsManager.getCommitInfo(cherryPickedHash.value());
    EXPECT_EQ(cherryPickedHash, commitsManager.getHeadCommitHash());
    EXPECT_NE(cherryPickedHash, secondCommitHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Initial commit");
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_drop)
{
    const auto cherryPicker = repository->CherryPicker();
    const auto commitsManager = repository->CommitsManager();


    const auto initialCommitHash = createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    const auto cherryPickedHash = cherryPicker.cherryPick(initialCommitHash, CppGit::CherryPickEmptyCommitStrategy::DROP);


    ASSERT_TRUE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash, std::string(40, '0'));
    EXPECT_EQ(secondCommitHash, commitsManager.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_stop)
{
    const auto cherryPicker = repository->CherryPicker();
    const auto commitsManager = repository->CommitsManager();


    const auto initialCommitHash = createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    const auto cherryPickedHash = cherryPicker.cherryPick(initialCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);


    ASSERT_FALSE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash.error(), CppGit::CherryPickResult::EMPTY_COMMIT_OR_EMPTY_DIFF);
    EXPECT_EQ(secondCommitHash, commitsManager.getHeadCommitHash());
    EXPECT_TRUE(cherryPicker.isCherryPickInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), initialCommitHash);
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_commitAfterStop)
{
    const auto cherryPicker = repository->CherryPicker();
    const auto commitsManager = repository->CommitsManager();


    const auto initialCommitHash = createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    cherryPicker.cherryPick(initialCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);
    const auto cherryPickedHash = cherryPicker.commitEmptyCherryPickedCommit();


    ASSERT_TRUE(cherryPickedHash.has_value());
    const auto cherryPickedInfo = commitsManager.getCommitInfo(cherryPickedHash.value());
    EXPECT_EQ(cherryPickedHash, commitsManager.getHeadCommitHash());
    EXPECT_NE(cherryPickedHash, initialCommitHash);
    EXPECT_NE(cherryPickedHash, secondCommitHash);
    checkCommitAuthorEqualTest(cherryPickedInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));
}

TEST_F(CherryPickTests, cherryPickDiffAlreadyExistFromAnotherCommitBranch)
{
    const auto cherryPicker = repository->CherryPicker();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    indexManager.add("file.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    branchesManager.changeBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    indexManager.add("file.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    const auto cherryPickedHash = cherryPicker.cherryPick(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);


    ASSERT_FALSE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash.error(), CppGit::CherryPickResult::EMPTY_COMMIT_OR_EMPTY_DIFF);
    EXPECT_EQ(thirdCommitHash, commitsManager.getHeadCommitHash());
    EXPECT_TRUE(cherryPicker.isCherryPickInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), secondCommitHash);
}

TEST_F(CherryPickTests, cherryPick_conflict_diffAlreadyExistButThenChanged)
{
    const auto cherryPicker = repository->CherryPicker();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified.");
    indexManager.add("file.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    branchesManager.changeBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified.");
    indexManager.add("file.txt");
    commitsManager.createCommit("Third commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    indexManager.add("file.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    const auto cherryPickedHash = cherryPicker.cherryPick(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);


    ASSERT_FALSE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash.error(), CppGit::CherryPickResult::CONFLICT);
    EXPECT_EQ(fourthCommitHash, commitsManager.getHeadCommitHash());
    EXPECT_TRUE(cherryPicker.isCherryPickInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified.\n>>>>>>> " + secondCommitHash + "\n");
}

TEST_F(CherryPickTests, cherryPick_conflict_resolve)
{
    const auto cherryPicker = repository->CherryPicker();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    indexManager.add("file.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    branchesManager.changeBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2");
    indexManager.add("file.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    cherryPicker.cherryPick(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);
    ASSERT_TRUE(cherryPicker.isCherryPickInProgress());

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Conflict resolved");
    indexManager.add("file.txt");

    const auto cherryPickResolvedHash = cherryPicker.continueCherryPick();


    ASSERT_TRUE(cherryPickResolvedHash.has_value());
    const auto cherryPickedInfo = commitsManager.getCommitInfo(cherryPickResolvedHash.value());
    EXPECT_EQ(cherryPickResolvedHash, commitsManager.getHeadCommitHash());
    EXPECT_NE(secondCommitHash, cherryPickResolvedHash);
    EXPECT_NE(thirdCommitHash, cherryPickResolvedHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Conflict resolved");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));
}

TEST_F(CherryPickTests, cherryPick_bothConflictAndNotFiles)
{
    const auto cherryPicker = repository->CherryPicker();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified.");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!");
    indexManager.add("file.txt");
    indexManager.add("file2.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    indexManager.add("file.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    const auto cherryPickedHash = cherryPicker.cherryPick(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);


    ASSERT_FALSE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash.error(), CppGit::CherryPickResult::CONFLICT);
    EXPECT_EQ(thirdCommitHash, commitsManager.getHeadCommitHash());
    EXPECT_TRUE(cherryPicker.isCherryPickInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified.\n>>>>>>> " + secondCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World 2!");
}

TEST_F(CherryPickTests, cherryPick_conflictTwoFiles)
{
    const auto cherryPicker = repository->CherryPicker();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    indexManager.add("file.txt");
    indexManager.add("file2.txt");
    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! Modified!");
    indexManager.add("file.txt");
    indexManager.add("file2.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! Modified 2!");
    indexManager.add("file.txt");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    const auto cherryPickedHash = cherryPicker.cherryPick(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);


    ASSERT_FALSE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash.error(), CppGit::CherryPickResult::CONFLICT);
    EXPECT_EQ(thirdCommitHash, commitsManager.getHeadCommitHash());
    EXPECT_TRUE(cherryPicker.isCherryPickInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2!\n=======\nHello, World! Modified!\n>>>>>>> " + secondCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "<<<<<<< HEAD\nHello, World! Modified 2!\n=======\nHello, World! Modified!\n>>>>>>> " + secondCommitHash + "\n");
}

TEST_F(CherryPickTests, cherryPick_abort)
{
    const auto cherryPicker = repository->CherryPicker();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    indexManager.add("file.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    branchesManager.changeBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2");
    indexManager.add("file.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    cherryPicker.cherryPick(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);

    cherryPicker.abortCherryPick();


    EXPECT_EQ(commitsManager.getHeadCommitHash(), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Modified 2");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));
}
