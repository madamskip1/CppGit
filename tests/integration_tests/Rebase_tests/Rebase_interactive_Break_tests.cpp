
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"
#include "RebaseFixture.hpp"
#include "RebaseTodoCommand.hpp"
#include "_details/FileUtility.hpp"

#include <gtest/gtest.h>

class RebaseInteractiveBreakTests : public RebaseFixture
{
};


TEST_F(RebaseInteractiveBreakTests, stop)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommitHash }, envp);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);

    auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
    // At that point we should only have Initial and Second Commits
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file2.txt"));

    auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                          + "break\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "stopped-sha"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "pick " + thirdCommitHash + " Third commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), thirdCommitHash);
}

TEST_F(RebaseInteractiveBreakTests, continueRebase)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);

    auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);

    auto continueBreakResult = rebase.continueRebase();


    ASSERT_TRUE(continueBreakResult.has_value());
    EXPECT_EQ(continueBreakResult.value(), thirdCommitHash); // we did FastForward
    EXPECT_EQ(commits.getHeadCommitHash(), thirdCommitHash);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[2].getHash(), thirdCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}


TEST_F(RebaseInteractiveBreakTests, fastForward_breakAfter)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommitHash }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);
    auto secondCommitInfo = commits.getCommitInfo(commitsLog[1].getHash());
    checkCommitAuthorEqualTest(secondCommitInfo);
    checkCommitCommiterEqualTest(secondCommitInfo);

    auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                          + "break\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "stopped-sha"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), secondCommitHash);
}

TEST_F(RebaseInteractiveBreakTests, noFastForward_breakAfter)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    auto secondCommitHash = commits.createCommit("Second commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto thirdCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Third commit", { secondCommitHash }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);
    EXPECT_NE(commits.getHeadCommitHash(), secondCommitHash);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommitHash);
    auto thirdCommitInfo = commits.getCommitInfo(commitsLog[1].getHash());
    checkCommitAuthorEqualTest(thirdCommitInfo);
    checkCommitCommiterNotEqualTest(thirdCommitInfo);

    auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                          + "pick " + thirdCommitHash + " Third commit\n"
                          + "break\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "stopped-sha"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), thirdCommitHash + " " + commits.getHeadCommitHash() + "\n");
}
