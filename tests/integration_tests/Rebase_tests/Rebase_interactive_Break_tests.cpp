
#include "RebaseFixture.hpp"

#include <CppGit/CommitsLogManager.hpp>
#include <CppGit/CommitsManager.hpp>
#include <CppGit/IndexManager.hpp>
#include <CppGit/RebaseTodoCommand.hpp>
#include <CppGit/Rebaser.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <gtest/gtest.h>

class RebaseInteractiveBreakTests : public RebaseFixture
{
};


TEST_F(RebaseInteractiveBreakTests, stop)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::BREAK);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);

    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file2.txt"));

    const auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                                + "break\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "pick " + thirdCommitHash + " Third commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseInteractiveBreakTests, continueRebase)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::BREAK);

    const auto continueBreakResult = rebaser.continueRebase();


    ASSERT_TRUE(continueBreakResult.has_value());
    EXPECT_EQ(continueBreakResult.value(), thirdCommitHash);
    EXPECT_EQ(commitsManager.getHeadCommitHash(), thirdCommitHash);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 3);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);
    EXPECT_EQ(log[2].getHash(), thirdCommitHash);

    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}


TEST_F(RebaseInteractiveBreakTests, fastForward_breakAfter)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::BREAK);
    EXPECT_EQ(commitsManager.getHeadCommitHash(), secondCommitHash);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);

    const auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                                + "break\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
}

TEST_F(RebaseInteractiveBreakTests, noFastForward_breakAfter)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", secondCommitHash);

    auto todoCommands = rebaser.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebaser.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::RebaseResult::BREAK);
    EXPECT_NE(commitsManager.getHeadCommitHash(), secondCommitHash);

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getMessage(), "Third commit");
    EXPECT_EQ(log[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[1]);

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
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
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), thirdCommitHash + " " + commitsManager.getHeadCommitHash() + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}
