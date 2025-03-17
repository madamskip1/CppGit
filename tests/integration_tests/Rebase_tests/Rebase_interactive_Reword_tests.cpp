#include "RebaseFixture.hpp"

#include <CppGit/BranchesManager.hpp>
#include <CppGit/CommitsLogManager.hpp>
#include <CppGit/CommitsManager.hpp>
#include <CppGit/IndexManager.hpp>
#include <CppGit/RebaseTodoCommand.hpp>
#include <CppGit/Rebaser.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <gtest/gtest.h>

class RebaseInteractiveRewordTests : public RebaseFixture
{
};


TEST_F(RebaseInteractiveRewordTests, fastForward_stop)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    indexManager.add("file.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", "Second commit description", initialCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::REWORD;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::REWORD);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");

    constexpr auto* expectedMessage = "Second commit\n\nSecond commit description";
    EXPECT_EQ(rebaser.getStoppedMessage(), expectedMessage);

    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commitsManager.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), "reword " + secondCommitHash + " Second commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(RebaseInteractiveRewordTests, noFastForward_stop)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    indexManager.add("file.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", secondCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[1].type = CppGit::RebaseTodoCommandType::REWORD;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::REWORD);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_NE(log[1].getHash(), thirdCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");

    constexpr auto* expectedMessage = "Third commit";
    EXPECT_EQ(rebaser.getStoppedMessage(), expectedMessage);

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "reword " + thirdCommitHash + " Third commit\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commitsManager.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseInteractiveRewordTests, fastForward_continue_changeMessage)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    indexManager.add("file.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::REWORD;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::REWORD);

    const auto continueRewordResult = rebaser.continueRebase("New message");


    ASSERT_TRUE(continueRewordResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), continueRewordResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), continueRewordResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "New message");
    EXPECT_EQ(log[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(RebaseInteractiveRewordTests, fastForward_continue_noChangeMessage)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);

    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    indexManager.add("file.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::REWORD;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::REWORD);

    const auto continueRewordResult = rebaser.continueRebase();


    ASSERT_TRUE(continueRewordResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), continueRewordResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), continueRewordResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Second commit");
    EXPECT_EQ(log[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(RebaseInteractiveRewordTests, noFastForward_continue_changeMesage)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    indexManager.add("file.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", secondCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[1].type = CppGit::RebaseTodoCommandType::REWORD;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::REWORD);

    const auto continueRewordResult = rebaser.continueRebase("New message");


    ASSERT_TRUE(continueRewordResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), continueRewordResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), continueRewordResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "New message");
    EXPECT_EQ(log[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseInteractiveRewordTests, noFastForward_continue_NoChangeMessage)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    indexManager.add("file.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", secondCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[1].type = CppGit::RebaseTodoCommandType::REWORD;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::REWORD);

    const auto continueRewordResult = rebaser.continueRebase();


    ASSERT_TRUE(continueRewordResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), continueRewordResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), continueRewordResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Third commit");
    EXPECT_EQ(log[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseInteractiveRewordTests, continue_changeMessageAndDescription)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    indexManager.add("file.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::REWORD;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::REWORD);

    const auto continueRewordResult = rebaser.continueRebase("New message", "New description");


    ASSERT_TRUE(continueRewordResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), continueRewordResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), continueRewordResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "New message");
    EXPECT_EQ(log[1].getDescription(), "New description");
    checkTestAuthorPreservedCommitterModified(log[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(RebaseInteractiveRewordTests, continue_keepOldMessage)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    const auto branchesManager = repository->BranchesManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    indexManager.add("file.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::REWORD;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::REWORD);

    const auto continueRewordResult = rebaser.continueRebase();


    ASSERT_TRUE(continueRewordResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), continueRewordResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), continueRewordResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Second commit");
    EXPECT_EQ(log[1].getHash(), continueRewordResult.value());

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(RebaseInteractiveRewordTests, breakAfter)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::REWORD;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::REWORD);

    const auto continueRewordResult = rebaser.continueRebase("New message");


    ASSERT_FALSE(continueRewordResult.has_value());
    EXPECT_EQ(continueRewordResult.error(), CppGit::RebaseResult::BREAK);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "New message");
    EXPECT_EQ(log[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[1]);

    const auto doneFileExpected = "reword " + secondCommitHash + " Second commit\n"
                                + "break\n";
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
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), secondCommitHash + " " + commitsManager.getHeadCommitHash() + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(RebaseInteractiveRewordTests, conflict_stop)
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
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::REWORD;

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

    constexpr auto* expectedMessage = "Fourth commit";
    EXPECT_EQ(rebaser.getStoppedMessage(), expectedMessage);

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "reword " + fourthCommitHash + " Fourth commit\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), thirdCommitHash + " " + log[1].getHash() + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveRewordTests, conflict_continue)
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
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file1.txt");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", "Fourth commit description", thirdCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::REWORD;

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
    ASSERT_EQ(log.size(), 3);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Third commit");
    EXPECT_EQ(log[1].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(log[1]);
    EXPECT_EQ(log[2].getMessage(), "Fourth commit");
    EXPECT_EQ(log[2].getDescription(), "Fourth commit description");
    checkTestAuthorPreservedCommitterModified(log[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveRewordTests, conflict_continue_changeMesssage)
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
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    indexManager.add("file1.txt");
    indexManager.add("file3.txt");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::REWORD;

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    indexManager.add("file1.txt");
    const auto continueRebaseResult = rebaser.continueRebase("New msg");


    ASSERT_TRUE(continueRebaseResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), continueRebaseResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), continueRebaseResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 3);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Third commit");
    EXPECT_EQ(log[1].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(log[1]);
    EXPECT_EQ(log[2].getMessage(), "New msg");
    EXPECT_EQ(log[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveRewordTests, conflict_breakAfter)
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
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::REWORD;
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
    ASSERT_EQ(log.size(), 3);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Third commit");
    EXPECT_EQ(log[1].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(log[1]);
    EXPECT_EQ(log[2].getMessage(), "Fourth commit");
    EXPECT_EQ(log[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "reword " + fourthCommitHash + " Fourth commit\n"
                                + "break\n";
    const auto rewrittenListExpected = thirdCommitHash + " " + log[1].getHash() + "\n"
                                     + fourthCommitHash + " " + log[2].getHash() + "\n";
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
