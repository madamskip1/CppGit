#include "../BaseRepositoryFixture.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"
#include "RebaseTodoCommand.hpp"
#include "_details/FileUtility.hpp"

#include <gtest/gtest.h>

class RebaseInteractiveDropTests : public BaseRepositoryFixture
{
};

TEST_F(RebaseInteractiveDropTests, drop_commit)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    index.add("file.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommit }, envp);
    auto thirdCommit = commits.createCommit("Third commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_TRUE(rebaseResult.has_value());
    EXPECT_NE(rebaseResult.value(), thirdCommit);
    EXPECT_NE(commits.getHeadCommitHash(), thirdCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file.txt"));
}

TEST_F(RebaseInteractiveDropTests, dropLeadToConflict_stop)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommit = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommit = commits.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file1.txt");
    index.add("file3.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto fourthCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Fourth commit", { thirdCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\n=======\nHello World 1, new!\n>>>>>>> " + fourthCommit + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_TRUE(std::filesystem::exists(gitRebaseDir));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "stopped-sha"), fourthCommit);
    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "pick " + thirdCommit + " Third commit\n"
                          + "pick " + fourthCommit + " Fourth commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    auto rewrittenListExpected = thirdCommit + " " + commitsLog[1].getHash() + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list"), rewrittenListExpected);
}

TEST_F(RebaseInteractiveDropTests, dropLeadToConflict_continue)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommit = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommit = commits.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file1.txt");
    index.add("file3.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto fourthCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Fourth commit", "Fourth commit description", { thirdCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;

    rebase.interactiveRebase(initialCommit, todoCommands);

    // Resolve conflict
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");

    auto continueRebaseResult = rebase.continueRebase();


    ASSERT_TRUE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.value(), commits.getHeadCommitHash());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_EQ(commitsLog[2].getMessage(), "Fourth commit");
    EXPECT_EQ(commitsLog[2].getDescription(), "Fourth commit description");
    EXPECT_NE(commitsLog[2].getHash(), fourthCommit);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}

TEST_F(RebaseInteractiveDropTests, dropLeadToConflict_breakAfterResolvedConflict)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommit = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommit = commits.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file1.txt");
    index.add("file3.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto fourthCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Fourth commit", { thirdCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    rebase.interactiveRebase(initialCommit, todoCommands); // now we are on conflict
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");

    auto continueRebaseResult = rebase.continueRebase(); // now we are on break


    ASSERT_FALSE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.error(), CppGit::Error::REBASE_BREAK);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_EQ(commitsLog[2].getMessage(), "Fourth commit");
    EXPECT_NE(commitsLog[2].getHash(), fourthCommit);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_TRUE(std::filesystem::exists(gitRebaseDir));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "pick " + thirdCommit + " Third commit\n"
                          + "pick " + fourthCommit + " Fourth commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    auto rewrittenListExpected = thirdCommit + " " + commitsLog[1].getHash() + "\n"
                               + fourthCommit + " " + commitsLog[2].getHash() + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list"), rewrittenListExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
}
