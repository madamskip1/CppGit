#include "RebaseFixture.hpp"

#include <CppGit/BranchesManager.hpp>
#include <CppGit/CommitsLogManager.hpp>
#include <CppGit/CommitsManager.hpp>
#include <CppGit/IndexManager.hpp>
#include <CppGit/RebaseTodoCommand.hpp>
#include <CppGit/Rebaser.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <gtest/gtest.h>

class RebaseInteractiveSquashTest : public RebaseFixture
{
};

TEST_F(RebaseInteractiveSquashTest, stop)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit", "Second commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::SQUASH);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file3.txt"));

    constexpr auto* expectedMessage = "Second commit\n\nSecond commit description\n\nThird commit\n\nThird commit description";
    EXPECT_EQ(rebaser.getStoppedMessage(), expectedMessage);

    const auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                                + "squash " + thirdCommitHash + " Third commit\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "pick " + fourthCommitHash + " Fourth commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-pending"), secondCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, twoSquashInARow_stop)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::SQUASH);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Second commit");
    EXPECT_EQ(log[1].getDescription(), "Third commit\n\nThird commit description");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    constexpr auto* expectedMessage = "Second commit\n\nThird commit\n\nThird commit description\n\nFourth commit";
    EXPECT_EQ(rebaser.getStoppedMessage(), expectedMessage);

    const auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                                + "squash " + thirdCommitHash + " Third commit\n"
                                + "squash " + fourthCommitHash + " Fourth commit\n";
    const auto rewrittenPendingExpected = secondCommitHash + "\n"
                                        + thirdCommitHash + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commitsManager.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "current-fixups"), "squash " + thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-pending"), rewrittenPendingExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, continue_changeMessage)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    createCommitWithTestAuthorCommiter("Second commit", "Second commit description", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::SQUASH);

    const auto squashContinueResult = rebaser.continueRebase("New message", "New description");


    ASSERT_TRUE(squashContinueResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), squashContinueResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), squashContinueResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 3);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "New message");
    EXPECT_EQ(log[1].getDescription(), "New description");
    checkTestAuthorPreservedCommitterModified(log[1]);
    EXPECT_EQ(log[2].getMessage(), "Fourth commit");
    EXPECT_EQ(log[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, continue_noChangeMessage)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    createCommitWithTestAuthorCommiter("Second commit", "Second commit description", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::SQUASH);

    const auto squashContinueResult = rebaser.continueRebase();


    ASSERT_TRUE(squashContinueResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), squashContinueResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), squashContinueResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 3);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Second commit");
    EXPECT_EQ(log[1].getDescription(), "Second commit description\n\nThird commit\n\nThird commit description");
    checkTestAuthorPreservedCommitterModified(log[1]);
    EXPECT_EQ(log[2].getMessage(), "Fourth commit");
    EXPECT_EQ(log[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, twoSquashInARow_continue)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    commitsManager.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::SQUASH);

    const auto squashContinueResult = rebaser.continueRebase();


    ASSERT_TRUE(squashContinueResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), squashContinueResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), squashContinueResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Second commit");
    EXPECT_EQ(log[1].getDescription(), "Third commit\n\nThird commit description\n\nFourth commit");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, breakAfter)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", "Second commit description", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands.emplace(todoCommands.begin() + 2, CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::SQUASH);

    const auto squashContinueResult = rebaser.continueRebase("New message", "New description");


    ASSERT_FALSE(squashContinueResult.has_value());
    EXPECT_EQ(squashContinueResult.error(), CppGit::RebaseResult::BREAK);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "New message");
    EXPECT_EQ(log[1].getDescription(), "New description");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file3.txt"));

    const auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                                + "squash " + thirdCommitHash + " Third commit\n"
                                + "break\n";
    const auto rewrittenListExpected = secondCommitHash + " " + log[1].getHash() + "\n"
                                     + thirdCommitHash + " " + log[1].getHash() + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "pick " + fourthCommitHash + " Fourth commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, breakAfter_noChangeMessage)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", "Second commit description", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands.emplace(todoCommands.begin() + 2, CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::SQUASH);

    const auto squashContinueResult = rebaser.continueRebase();


    ASSERT_FALSE(squashContinueResult.has_value());
    EXPECT_EQ(squashContinueResult.error(), CppGit::RebaseResult::BREAK);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Second commit");
    EXPECT_EQ(log[1].getDescription(), "Second commit description\n\nThird commit\n\nThird commit description");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file3.txt"));

    const auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                                + "squash " + thirdCommitHash + " Third commit\n"
                                + "break\n";
    const auto rewrittenListExpected = secondCommitHash + " " + log[1].getHash() + "\n"
                                     + thirdCommitHash + " " + log[1].getHash() + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "pick " + fourthCommitHash + " Fourth commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, twoSquashInARow_breakAfter)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::SQUASH);

    const auto squashContinueResult = rebaser.continueRebase();


    ASSERT_FALSE(squashContinueResult.has_value());
    EXPECT_EQ(squashContinueResult.error(), CppGit::RebaseResult::BREAK);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Second commit");
    EXPECT_EQ(log[1].getDescription(), "Third commit\n\nThird commit description\n\nFourth commit");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    const auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                                + "squash " + thirdCommitHash + " Third commit\n"
                                + "squash " + fourthCommitHash + " Fourth commit\n"
                                + "break\n";
    const auto rewrittenListExpected = secondCommitHash + " " + log[1].getHash() + "\n"
                                     + thirdCommitHash + " " + log[1].getHash() + "\n"
                                     + fourthCommitHash + " " + log[1].getHash() + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, squashAfterBreak_breakAfter_changeMessage)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", "Second commit description", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands.emplace(todoCommands.begin() + 3, CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::BREAK);

    const auto breakContinueResult = rebaser.continueRebase();
    ASSERT_FALSE(breakContinueResult.has_value());
    EXPECT_EQ(breakContinueResult.error(), CppGit::RebaseResult::SQUASH);

    const auto squashContinueResult = rebaser.continueRebase("New message", "New description");


    ASSERT_FALSE(squashContinueResult.has_value());
    EXPECT_EQ(squashContinueResult.error(), CppGit::RebaseResult::BREAK);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "New message");
    EXPECT_EQ(log[1].getDescription(), "New description");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file3.txt"));

    const auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                                + "break\n"
                                + "squash " + thirdCommitHash + " Third commit\n"
                                + "break\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "pick " + fourthCommitHash + " Fourth commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), thirdCommitHash + " " + log[1].getHash() + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, squashAfterBreak_breakAfter_noChangeMessage)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", "Second commit description", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands.emplace(todoCommands.begin() + 3, CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::BREAK);

    const auto breakContinueResult = rebaser.continueRebase();
    ASSERT_FALSE(breakContinueResult.has_value());
    EXPECT_EQ(breakContinueResult.error(), CppGit::RebaseResult::SQUASH);

    const auto squashContinueResult = rebaser.continueRebase();


    ASSERT_FALSE(squashContinueResult.has_value());
    EXPECT_EQ(squashContinueResult.error(), CppGit::RebaseResult::BREAK);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Second commit");
    EXPECT_EQ(log[1].getDescription(), "Second commit description\n\nThird commit\n\nThird commit description");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file3.txt"));

    const auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                                + "break\n"
                                + "squash " + thirdCommitHash + " Third commit\n"
                                + "break\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "pick " + fourthCommitHash + " Fourth commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), thirdCommitHash + " " + log[1].getHash() + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, twoSquashInARowAfterBreak_breakAfter)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::BREAK);

    const auto breakContinueResult = rebaser.continueRebase();
    ASSERT_FALSE(breakContinueResult.has_value());
    EXPECT_EQ(breakContinueResult.error(), CppGit::RebaseResult::SQUASH);

    const auto squashContinueResult = rebaser.continueRebase();


    ASSERT_FALSE(squashContinueResult.has_value());
    EXPECT_EQ(squashContinueResult.error(), CppGit::RebaseResult::BREAK);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Second commit");
    EXPECT_EQ(log[1].getDescription(), "Third commit\n\nThird commit description\n\nFourth commit");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    const auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                                + "break\n"
                                + "squash " + thirdCommitHash + " Third commit\n"
                                + "squash " + fourthCommitHash + " Fourth commit\n"
                                + "break\n";
    const auto rewrittenListExpected = thirdCommitHash + " " + log[1].getHash() + "\n"
                                     + fourthCommitHash + " " + log[1].getHash() + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, conflictOnLastSquash_stop)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file1.txt");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Third commit");
    EXPECT_EQ(log[1].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\n=======\nHello World 1, new!\n>>>>>>> " + fourthCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    constexpr auto* expectedMessage = "Third commit\n\nThird commit description\n\nFourth commit";
    EXPECT_EQ(rebaser.getStoppedMessage(), expectedMessage);

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "squash " + fourthCommitHash + " Fourth commit\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commitsManager.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "current-fixups"), "squash " + fourthCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-pending"), thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, conflictOnLastSquash_continue)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    createCommitWithTestAuthorCommiter("Third commit", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file1.txt");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit", "Fourth commit description");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    indexManager.add("file1.txt");
    const auto continueRebaseResult = rebaser.continueRebase();


    ASSERT_TRUE(continueRebaseResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), continueRebaseResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), continueRebaseResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Third commit");
    EXPECT_EQ(log[1].getDescription(), "Fourth commit\n\nFourth commit description");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, conflictOnLastSquash_breakAfter)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file1.txt");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    indexManager.add("file1.txt");
    const auto continueRebaseResult = rebaser.continueRebase();


    ASSERT_FALSE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.error(), CppGit::RebaseResult::BREAK);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Third commit");
    EXPECT_EQ(log[1].getDescription(), "Fourth commit");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "squash " + fourthCommitHash + " Fourth commit\n"
                                + "break\n";
    const auto rewrittenListExpected = thirdCommitHash + " " + log[1].getHash() + "\n"
                                     + fourthCommitHash + " " + log[1].getHash() + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, conflictOnNotLastSquash_stop)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file1.txt");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    indexManager.add("file4.txt");
    const auto fifthCommitHash = commitsManager.createCommit("Fifth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::SQUASH;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Third commit");
    EXPECT_EQ(log[1].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\n=======\nHello World 1, new!\n>>>>>>> " + fourthCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file4.txt"));

    constexpr auto* expectedMessage = "Third commit\n\nThird commit description\n\nFourth commit";
    EXPECT_EQ(rebaser.getStoppedMessage(), expectedMessage);

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "squash " + fourthCommitHash + " Fourth commit\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commitsManager.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "current-fixups"), "squash " + fourthCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "squash " + fifthCommitHash + " Fifth commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-pending"), thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fifthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, conflictOnNotLastSquash_continueToLastSquash)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file1.txt");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit", "Fourth commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    indexManager.add("file4.txt");
    const auto fifthCommitHash = commitsManager.createCommit("Fifth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::SQUASH;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    indexManager.add("file1.txt");
    const auto continueRebaseResult = rebaser.continueRebase();


    ASSERT_FALSE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.error(), CppGit::RebaseResult::SQUASH);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Third commit");
    EXPECT_EQ(log[1].getDescription(), "Third commit description\n\nFourth commit\n\nFourth commit description");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    constexpr auto* expectedMessage = "Third commit\n\nThird commit description\n\nFourth commit\n\nFourth commit description\n\nFifth commit";
    EXPECT_EQ(rebaser.getStoppedMessage(), expectedMessage);

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "squash " + fourthCommitHash + " Fourth commit\n"
                                + "squash " + fifthCommitHash + " Fifth commit\n";
    const auto rewrittenPendingExpected = thirdCommitHash + "\n"
                                        + fourthCommitHash + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commitsManager.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-pending"), rewrittenPendingExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fifthCommitHash);
}

TEST_F(RebaseInteractiveSquashTest, conflictOnNotLastSquash_continueToEnd)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    createCommitWithTestAuthorCommiter("Third commit", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file1.txt");
    indexManager.add("file3.txt");
    commitsManager.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    indexManager.add("file4.txt");
    const auto fifthCommitHash = commitsManager.createCommit("Fifth commit", "Fifth commit description");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::SQUASH;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    indexManager.add("file1.txt");
    const auto conflictContinueResult = rebaser.continueRebase();
    ASSERT_FALSE(conflictContinueResult.has_value());
    EXPECT_EQ(conflictContinueResult.error(), CppGit::RebaseResult::SQUASH);

    const auto continueRebaseResult = rebaser.continueRebase();


    ASSERT_TRUE(continueRebaseResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), continueRebaseResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), continueRebaseResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Third commit");
    EXPECT_EQ(log[1].getDescription(), "Fourth commit\n\nFifth commit\n\nFifth commit description");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fifthCommitHash);
}


TEST_F(RebaseInteractiveSquashTest, conflictOnNotLastSquash_breakAfter)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file1.txt");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    indexManager.add("file4.txt");
    const auto fifthCommitHash = commitsManager.createCommit("Fifth commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    indexManager.add("file1.txt");
    const auto conflictContinueResult = rebaser.continueRebase();
    ASSERT_FALSE(conflictContinueResult.has_value());
    EXPECT_EQ(conflictContinueResult.error(), CppGit::RebaseResult::SQUASH);

    const auto continueRebaseResult = rebaser.continueRebase();


    ASSERT_FALSE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.error(), CppGit::RebaseResult::BREAK);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Third commit");
    EXPECT_EQ(log[1].getDescription(), "Third commit description\n\nFourth commit\n\nFifth commit");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "squash " + fourthCommitHash + " Fourth commit\n"
                                + "squash " + fifthCommitHash + " Fifth commit\n"
                                + "break\n";
    const auto rewrittenListExpected = thirdCommitHash + " " + log[1].getHash() + "\n"
                                     + fourthCommitHash + " " + log[1].getHash() + "\n"
                                     + fifthCommitHash + " " + log[1].getHash() + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fifthCommitHash);
}
