
#include "../BaseRepositoryFixture.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"
#include "RebaseTodoCommand.hpp"
#include "_details/FileUtility.hpp"

#include <gtest/gtest.h>

class RebaseInteractiveBreakTests : public BaseRepositoryFixture
{
};


TEST_F(RebaseInteractiveBreakTests, stop)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    auto secondCommit = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommit);
    // At that point we should only have Initial and Second Commits
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file2.txt"));
    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_TRUE(std::filesystem::exists(gitRebaseDir));
    auto todoFileExpected = "pick " + thirdCommitHash + " Third commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), todoFileExpected);
    auto doneFileExpected = "pick " + secondCommit + " Second commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "rewritten-list"));
}

TEST_F(RebaseInteractiveBreakTests, continueRebase)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    auto secondCommit = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);

    auto continueBreakResult = rebase.continueRebase();


    ASSERT_TRUE(continueBreakResult.has_value());
    EXPECT_EQ(continueBreakResult.value(), thirdCommitHash); // we did FastForward
    EXPECT_EQ(commits.getHeadCommitHash(), thirdCommitHash);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[2].getHash(), thirdCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));
    EXPECT_FALSE(std::filesystem::exists(repository->getGitDirectoryPath() / "rebase-merge"));
}


TEST_F(RebaseInteractiveBreakTests, fastForward_breakAfter)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    auto secondCommitInfo = commits.getCommitInfo(commitsLog[1].getHash());
    checkCommitAuthorEqualTest(secondCommitInfo);
    checkCommitCommiterEqualTest(secondCommitInfo);
    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "pick " + secondCommit + " Second commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "rewritten-list"));
}

TEST_F(RebaseInteractiveBreakTests, noFastForward_breakAfter)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    auto secondCommit = commits.createCommit("Second commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto thirdCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Third commit", { secondCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);
    EXPECT_NE(commits.getHeadCommitHash(), secondCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    auto thirdCommitInfo = commits.getCommitInfo(commitsLog[1].getHash());
    checkCommitAuthorEqualTest(thirdCommitInfo);
    checkCommitCommiterNotEqualTest(thirdCommitInfo);
    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), thirdCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "pick " + thirdCommit + " Third commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
    auto rewrittenListExpected = thirdCommit + " " + commits.getHeadCommitHash() + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list"), rewrittenListExpected);
}
