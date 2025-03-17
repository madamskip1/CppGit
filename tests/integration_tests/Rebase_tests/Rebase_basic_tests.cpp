
#include "RebaseFixture.hpp"

#include <CppGit/BranchesManager.hpp>
#include <CppGit/CommitsLogManager.hpp>
#include <CppGit/CommitsManager.hpp>
#include <CppGit/IndexManager.hpp>
#include <CppGit/Rebaser.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <gtest/gtest.h>

class RebaseBasicTests : public RebaseFixture
{
};

TEST_F(RebaseBasicTests, simpleRebase)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    const auto rebaseResult = rebaser.rebase("main");


    ASSERT_TRUE(rebaseResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), rebaseResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/second_branch");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), rebaseResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 4);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);
    EXPECT_EQ(log[2].getMessage(), "Third commit");
    EXPECT_EQ(log[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[2]);
    EXPECT_EQ(log[3].getMessage(), "Fourth commit");
    EXPECT_EQ(log[3].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[3]);

    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseBasicTests, conflictOnFirstCommit_stop)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    indexManager.add("file.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second2");
    indexManager.add("file.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    const auto rebaseResult = rebaser.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nMain\n=======\nSecond\n>>>>>>> " + thirdCommitHash + "\n");

    constexpr auto* expectedMessage = "Third commit\n\nThird commit description";
    EXPECT_EQ(rebaser.getStoppedMessage(), expectedMessage);

    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), "pick " + thirdCommitHash + " Third commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "pick " + fourthCommitHash + " Fourth commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseBasicTests, conflictOnNotFirstCommit_stop)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    indexManager.add("file.txt");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    const auto rebaseResult = rebaser.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 3);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);
    EXPECT_EQ(log[2].getMessage(), "Third commit");
    EXPECT_EQ(log[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nMain\n=======\nSecond\n>>>>>>> " + fourthCommitHash + "\n");

    constexpr auto* expectedMessage = "Fourth commit";
    EXPECT_EQ(rebaser.getStoppedMessage(), expectedMessage);

    const auto doneFileExpected = "pick " + thirdCommitHash + " Third commit\n"
                                + "pick " + fourthCommitHash + " Fourth commit\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), thirdCommitHash + " " + log[2].getHash() + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseBasicTests, conflict_bothConflictedAndNotFiles_stop)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    indexManager.add("file1.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!");
    indexManager.add("file1.txt");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);

    const auto rebaseResult = rebaser.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\nHello, World! Modified 1!\n=======\nHello, World! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World 2!");

    constexpr auto* expectedMessage = "Third commit";
    EXPECT_EQ(rebaser.getStoppedMessage(), expectedMessage);

    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), "pick " + thirdCommitHash + " Third commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), "Third commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseBasicTests, conflict_conflictedTwoFiles_stop)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!");
    indexManager.add("file1.txt");
    indexManager.add("file2.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 1!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2! Modified 1!");
    indexManager.add("file1.txt");
    indexManager.add("file2.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2! Modified 2!");
    indexManager.add("file1.txt");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);

    const auto rebaseResult = rebaser.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\nHello, World! Modified 1!\n=======\nHello, World! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "<<<<<<< HEAD\nHello, World 2! Modified 1!\n=======\nHello, World 2! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");

    constexpr auto* expectedMessage = "Third commit";
    EXPECT_EQ(rebaser.getStoppedMessage(), expectedMessage);

    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), "pick " + thirdCommitHash + " Third commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseBasicTests, conflict_abort)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();


    createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    branchesManager.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    indexManager.add("file.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    const auto rebaseResult = rebaser.rebase("main");
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    rebaser.abortRebase();


    EXPECT_EQ(commitsManager.getHeadCommitHash(), thirdCommitHash);
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/second_branch");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), thirdCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Second");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseBasicTests, conflict_continue)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    indexManager.add("file.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", initialCommitHash);

    const auto rebaseResult = rebaser.rebase("main");
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Resolved");
    indexManager.add("file.txt");
    const auto rebaseContinueResult = rebaser.continueRebase();


    ASSERT_TRUE(rebaseContinueResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), rebaseContinueResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/second_branch");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), rebaseContinueResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 3);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);
    EXPECT_EQ(log[2].getMessage(), "Third commit");
    EXPECT_EQ(log[2].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(log[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Resolved");


    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseBasicTests, skipPreviouslyAppliedCommit)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Line 1");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");

    branchesManager.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Line changed");
    indexManager.add("file.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Line changed");
    indexManager.add("file.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Line changed again");
    indexManager.add("file.txt");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    const auto rebaseResult = rebaser.rebase("main");


    ASSERT_TRUE(rebaseResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), rebaseResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/second_branch");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), rebaseResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 3);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);
    EXPECT_EQ(log[2].getMessage(), "Fourth commit");
    EXPECT_EQ(log[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Line changed again");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseBasicTests, dontSkipEmptyCommit)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    indexManager.add("file.txt");
    const auto initialCommitHash = commitsManager.createCommit("Initial commit");

    branchesManager.createBranch("second_branch");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");

    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World 3!");
    indexManager.add("file.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    const auto rebaseResult = rebaser.rebase("main");


    ASSERT_TRUE(rebaseResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), rebaseResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/second_branch");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), rebaseResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 4);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);
    EXPECT_EQ(log[2].getMessage(), "Third commit");
    EXPECT_EQ(log[2].getDescription(), "");
    EXPECT_EQ(log[3].getMessage(), "Fourth commit");
    EXPECT_EQ(log[3].getDescription(), "");
    // checkTestAuthorPreservedCommitterModified(log[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}
