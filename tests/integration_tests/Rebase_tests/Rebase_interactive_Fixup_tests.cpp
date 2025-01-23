#include "../BaseRepositoryFixture.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"
#include "RebaseTodoCommand.hpp"
#include "_details/FileUtility.hpp"
#include "_details/Parser/Parser.hpp"

#include <gtest/gtest.h>
#include <regex>

class RebaseInteractiveFixupTest : public BaseRepositoryFixture
{
};

TEST_F(RebaseInteractiveFixupTest, interactive_fixup)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommit }, envp);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommit = commits.createCommit("Third commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::FIXUP;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    EXPECT_NE(commits.getHeadCommitHash(), secondCommit);
    EXPECT_NE(commits.getHeadCommitHash(), thirdCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_NE(commitsLog[2].getHash(), thirdCommit);
    EXPECT_NE(commitsLog[2].getHash(), fourthCommit);
    EXPECT_EQ(commitsLog[2].getMessage(), "Fourth commit");
    checkCommitAuthorEqualTest(commits.getCommitInfo(commitsLog[1].getHash()));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
}

TEST_F(RebaseInteractiveFixupTest, interactive_fixup_twoInARow)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommit }, envp);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommit = commits.createCommit("Third commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    auto fifthCommit = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    EXPECT_NE(commits.getHeadCommitHash(), secondCommit);
    EXPECT_NE(commits.getHeadCommitHash(), thirdCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_NE(commitsLog[2].getHash(), thirdCommit);
    EXPECT_NE(commitsLog[2].getHash(), fourthCommit);
    EXPECT_NE(commitsLog[2].getHash(), fifthCommit);
    EXPECT_EQ(commitsLog[2].getMessage(), "Fifth commit");
    checkCommitAuthorEqualTest(commits.getCommitInfo(commitsLog[1].getHash()));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");
}

TEST_F(RebaseInteractiveFixupTest, interactive_breakAfterFixup_noRewrittenCommitsBefore)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommit }, envp);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommit = commits.createCommit("Third commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    auto fifthCommit = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace(todoCommands.begin() + 3, CppGit::RebaseTodoCommandType::BREAK);

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);
    EXPECT_NE(commits.getHeadCommitHash(), fourthCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_NE(commitsLog[1].getHash(), fourthCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commits.getHeadCommitHash(), commitsLog[1].getHash());

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file4.txt"));

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), fifthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fifthCommit);
    auto rebaseTodoExpected = "pick " + fifthCommit + " Fifth commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), rebaseTodoExpected);
    auto doneFileExpected = "pick " + secondCommit + " Second commit\n"
                          + "fixup " + thirdCommit + " Third commit\n"
                          + "fixup " + fourthCommit + " Fourth commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
    auto rewrittenListExpected = secondCommit + " " + commits.getHeadCommitHash() + "\n"
                               + thirdCommit + " " + commits.getHeadCommitHash() + "\n"
                               + fourthCommit + " " + commits.getHeadCommitHash() + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list"), rewrittenListExpected);
}

TEST_F(RebaseInteractiveFixupTest, interactive_breakAfterFixup_rewrittenCommitsBefore)
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
    auto thirdCommit = commits.createCommit("Third commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto fifthCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Fifth commit", { fourthCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace(todoCommands.begin() + 4, CppGit::RebaseTodoCommandType::BREAK);

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);
    EXPECT_NE(commits.getHeadCommitHash(), fourthCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_NE(commitsLog[2].getHash(), fourthCommit);
    EXPECT_NE(commitsLog[2].getHash(), fifthCommit);
    EXPECT_EQ(commitsLog[2].getMessage(), "Fourth commit");
    EXPECT_EQ(commits.getHeadCommitHash(), commitsLog[2].getHash());

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), fifthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fifthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "pick " + thirdCommit + " Third commit\n"
                          + "pick " + fourthCommit + " Fourth commit\n"
                          + "fixup " + fifthCommit + " Fifth commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
    auto rewrittenListExpected = thirdCommit + " " + commitsLog[1].getHash() + "\n"
                               + fourthCommit + " " + commits.getHeadCommitHash() + "\n"
                               + fifthCommit + " " + commits.getHeadCommitHash() + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list"), rewrittenListExpected);
}

TEST_F(RebaseInteractiveFixupTest, interactive_fixupAfterBreak_noRewrittenCommitsBefore)
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
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto thirdCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Third commit", { secondCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    rebase.interactiveRebase(initialCommit, todoCommands); // stopped at first break
    auto rebaseResult = rebase.continueRebase();


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);
    EXPECT_NE(commits.getHeadCommitHash(), thirdCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commits.getHeadCommitHash(), commitsLog[1].getHash());

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), thirdCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "pick " + secondCommit + " Second commit\n"
                          + "break\n"
                          + "fixup " + thirdCommit + " Third commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
    auto rewrittenListExpected = thirdCommit + " " + commitsLog[1].getHash() + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list"), rewrittenListExpected);
}

TEST_F(RebaseInteractiveFixupTest, interactive_fixupAfterBreak_rewrittenCommitsBefore)
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
    auto thirdCommit = commits.createCommit("Third commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto fifthCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Fifth commit", { fourthCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace(todoCommands.begin() + 3, CppGit::RebaseTodoCommandType::BREAK);
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    rebase.interactiveRebase(initialCommit, todoCommands); // stopped at first break
    auto rebaseResult = rebase.continueRebase();


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);
    EXPECT_NE(commits.getHeadCommitHash(), thirdCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_NE(commitsLog[2].getHash(), fourthCommit);
    EXPECT_NE(commitsLog[2].getHash(), fifthCommit);
    EXPECT_EQ(commitsLog[2].getMessage(), "Fourth commit");
    EXPECT_EQ(commits.getHeadCommitHash(), commitsLog[2].getHash());

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), fifthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fifthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "pick " + thirdCommit + " Third commit\n"
                          + "pick " + fourthCommit + " Fourth commit\n"
                          + "break\n"
                          + "fixup " + fifthCommit + " Fifth commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
    auto rewrittenList = CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list");
    auto splittedRewrittenList = CppGit::Parser::splitToStringViewsVector(rewrittenList, "\n");
    EXPECT_EQ(splittedRewrittenList[0], thirdCommit + " " + commitsLog[1].getHash());
    EXPECT_NE(splittedRewrittenList[1], fourthCommit + " " + commitsLog[2].getHash());
    EXPECT_TRUE(std::regex_match(std::string{ splittedRewrittenList[1] }, std::regex{ "(^" + fourthCommit + " " + R"([\d\w]{40})$)" }));
    EXPECT_EQ(splittedRewrittenList[2], fifthCommit + " " + commitsLog[2].getHash());
}

TEST_F(RebaseInteractiveFixupTest, interactive_fixupAfterBreak_pickAsFirstRewritten)
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
    auto thirdCommit = commits.createCommit("Third commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto fourthCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Fourth commit", { thirdCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace(todoCommands.begin() + 2, CppGit::RebaseTodoCommandType::BREAK);
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    rebase.interactiveRebase(initialCommit, todoCommands); // stopped at first break
    auto rebaseResult = rebase.continueRebase();


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);
    EXPECT_NE(commits.getHeadCommitHash(), thirdCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_NE(commitsLog[1].getHash(), fourthCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commits.getHeadCommitHash(), commitsLog[1].getHash());

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "pick " + thirdCommit + " Third commit\n"
                          + "break\n"
                          + "fixup " + fourthCommit + " Fourth commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
    auto rewrittenList = CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list");
    auto splittedRewrittenList = CppGit::Parser::splitToStringViewsVector(rewrittenList, "\n");
    EXPECT_NE(splittedRewrittenList[0], thirdCommit + " " + commitsLog[1].getHash());
    EXPECT_TRUE(std::regex_match(std::string{ splittedRewrittenList[0] }, std::regex{ "(^" + thirdCommit + " " + R"([\d\w]{40})$)" }));
    EXPECT_EQ(splittedRewrittenList[1], fourthCommit + " " + commitsLog[1].getHash());
}

TEST_F(RebaseInteractiveFixupTest, interactive_conflictDuringFixup_lastFixup_stop)
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
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;

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
                          + "fixup " + fourthCommit + " Fourth commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "rebase-merge" / "amend"), commitsLog[1].getHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "rebase-merge" / "current-fixup"), "fixup " + fourthCommit + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "rebase-merge" / "rewritten-pending"), thirdCommit + "\n");
}

TEST_F(RebaseInteractiveFixupTest, interactive_conflictDuringFixup_lastFixup_continue)
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
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;

    rebase.interactiveRebase(initialCommit, todoCommands);

    // Resolve conflict
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");

    auto continueRebaseResult = rebase.continueRebase();


    ASSERT_TRUE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.value(), commits.getHeadCommitHash());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getDescription(), "");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}

TEST_F(RebaseInteractiveFixupTest, interactive_conflictDuringFixup_lastFixup_breakAfterContinue)
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
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    rebase.interactiveRebase(initialCommit, todoCommands); // now we are on conflict
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");

    auto continueRebaseResult = rebase.continueRebase(); // now we are on break


    ASSERT_FALSE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.error(), CppGit::Error::REBASE_BREAK);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_TRUE(std::filesystem::exists(gitRebaseDir));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "pick " + thirdCommit + " Third commit\n"
                          + "fixup " + fourthCommit + " Fourth commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    auto rewrittenListExpected = thirdCommit + " " + commitsLog[1].getHash() + "\n"
                               + fourthCommit + " " + commitsLog[1].getHash() + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list"), rewrittenListExpected);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge" / "amend"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge" / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge" / "rewritten-pending"));
}

TEST_F(RebaseInteractiveFixupTest, interactive_conflictDuringFixup_notLastFixup_stop)
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
    auto fourthCommit = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    auto fifthCommit = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;

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
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file4.txt"));

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_TRUE(std::filesystem::exists(gitRebaseDir));
    auto todoFileExpected = "fixup " + fifthCommit + " Fifth commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), todoFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "stopped-sha"), fourthCommit);
    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "pick " + thirdCommit + " Third commit\n"
                          + "fixup " + fourthCommit + " Fourth commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "rebase-merge" / "amend"), commitsLog[1].getHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "rebase-merge" / "current-fixup"), "fixup " + fourthCommit + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "rebase-merge" / "rewritten-pending"), thirdCommit + "\n");
}

TEST_F(RebaseInteractiveFixupTest, interactive_conflictDuringFixup_notLastFixup_continue)
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
    auto fourthCommit = commits.createCommit("Fourth commit", "Fourth commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    auto fifthCommit = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;

    rebase.interactiveRebase(initialCommit, todoCommands);

    // Resolve conflict
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");

    auto continueRebaseResult = rebase.continueRebase();


    ASSERT_TRUE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.value(), commits.getHeadCommitHash());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getDescription(), "");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}

TEST_F(RebaseInteractiveFixupTest, interactive_conflictDuringFixup_notLastFixup_breakAfterContinue)
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
    auto fourthCommit = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    auto fifthCommit = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    rebase.interactiveRebase(initialCommit, todoCommands); // now we are on conflict
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");

    auto continueRebaseResult = rebase.continueRebase(); // now we are on break


    ASSERT_FALSE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.error(), CppGit::Error::REBASE_BREAK);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_TRUE(std::filesystem::exists(gitRebaseDir));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "pick " + thirdCommit + " Third commit\n"
                          + "fixup " + fourthCommit + " Fourth commit\n"
                          + "fixup " + fifthCommit + " Fifth commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    auto rewrittenListExpected = thirdCommit + " " + commitsLog[1].getHash() + "\n"
                               + fourthCommit + " " + commitsLog[1].getHash() + "\n"
                               + fifthCommit + " " + commitsLog[1].getHash() + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list"), rewrittenListExpected);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge" / "amend"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge" / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge" / "rewritten-pending"));
}
