#include "../BaseRepositoryFixture.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"
#include "RebaseTodoCommand.hpp"
#include "_details/FileUtility.hpp"

#include <gtest/gtest.h>

class RebaseInteractiveSquashFixupTest : public BaseRepositoryFixture
{
};

TEST_F(RebaseInteractiveSquashFixupTest, interactive_squashFixup_stop)
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
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_SQUASH);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_NE(commitsLog[1].getHash(), fourthCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commits.getHeadCommitHash(), commitsLog[1].getHash());
    auto messageExpected = "Second commit\n\nThird commit\n\nThird commit description";
    EXPECT_EQ(rebase.getStoppedMessage(), messageExpected);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "pick " + secondCommit + " Second commit\n"
                          + "squash " + thirdCommit + " Third commit\n"
                          + "fixup " + fourthCommit + " Fourth commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "rewritten-list"));
    auto rewrittenPendingExpected = secondCommit + "\n"
                                  + thirdCommit + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-pending"), rewrittenPendingExpected);
    auto currentFixupExpected = "squash " + thirdCommit + "\n"
                              + "fixup " + fourthCommit + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "current-fixup"), currentFixupExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "message"), messageExpected);
}

TEST_F(RebaseInteractiveSquashFixupTest, interactive_squashFixup_continue)
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
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;

    rebase.interactiveRebase(initialCommit, todoCommands);

    auto squashContinueResult = rebase.continueRebase();


    ASSERT_TRUE(squashContinueResult.has_value());
    EXPECT_EQ(squashContinueResult.value(), commits.getHeadCommitHash());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit\n\nThird commit description");
    EXPECT_EQ(commits.getHeadCommitHash(), commitsLog[1].getHash());

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}

TEST_F(RebaseInteractiveSquashFixupTest, interactive_breakAfterSquashFixup)
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
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    rebase.interactiveRebase(initialCommit, todoCommands);

    auto squashContinueResult = rebase.continueRebase();


    ASSERT_FALSE(squashContinueResult.has_value());
    EXPECT_EQ(squashContinueResult.error(), CppGit::Error::REBASE_BREAK);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_NE(commitsLog[1].getHash(), fourthCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit\n\nThird commit description");

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "pick " + secondCommit + " Second commit\n"
                          + "squash " + thirdCommit + " Third commit\n"
                          + "fixup " + fourthCommit + " Fourth commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
    auto rewrittenListExpected = secondCommit + " " + commitsLog[1].getHash() + "\n"
                               + thirdCommit + " " + commitsLog[1].getHash() + "\n"
                               + fourthCommit + " " + commitsLog[1].getHash() + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list"), rewrittenListExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "rewritten-pending"));
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "message"));
}

TEST_F(RebaseInteractiveSquashFixupTest, interactive_squashFixupAfterBreak)
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
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    rebase.interactiveRebase(initialCommit, todoCommands);
    rebase.continueRebase(); // we was at first break

    auto squashContinueResult = rebase.continueRebase();


    ASSERT_FALSE(squashContinueResult.has_value());
    EXPECT_EQ(squashContinueResult.error(), CppGit::Error::REBASE_BREAK);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_NE(commitsLog[1].getHash(), fourthCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit\n\nThird commit description");

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "pick " + secondCommit + " Second commit\n"
                          + "break\n"
                          + "squash " + thirdCommit + " Third commit\n"
                          + "fixup " + fourthCommit + " Fourth commit\n"
                          + "break\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "stopped-sha"));
    auto rewrittenListExpected = thirdCommit + " " + commitsLog[1].getHash() + "\n"
                               + fourthCommit + " " + commitsLog[1].getHash() + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list"), rewrittenListExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "rewritten-pending"));
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "message"));
}

TEST_F(RebaseInteractiveSquashFixupTest, interactive_conflictDuringSquashFixup_onFixup_stop)
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
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file1.txt");
    index.add("file4.txt");
    auto fifthCommit = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description\n\nFourth commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\n=======\nHello World 1, new!\n>>>>>>> " + fifthCommit + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_TRUE(std::filesystem::exists(gitRebaseDir));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "stopped-sha"), fifthCommit);
    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "pick " + thirdCommit + " Third commit\n"
                          + "squash " + fourthCommit + " Fourth commit\n"
                          + "fixup " + fifthCommit + " Fifth commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "rebase-merge" / "amend"), commitsLog[1].getHash());
    auto currentFixupExpected = "squash " + fourthCommit + "\n"
                              + "fixup " + fifthCommit + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "rebase-merge" / "current-fixup"), currentFixupExpected);
    auto rewrittenPendingExpected = thirdCommit + "\n"
                                  + fourthCommit + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "rebase-merge" / "rewritten-pending"), rewrittenPendingExpected);
}

TEST_F(RebaseInteractiveSquashFixupTest, interactive_conflictDuringSquashFixup_onFixup_continue)
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
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommit = commits.createCommit("Fourth commit", "Fourth commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file1.txt");
    index.add("file4.txt");
    auto fifthCommit = commits.createCommit("Fifth commit", "Fifth commit description");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;

    rebase.interactiveRebase(initialCommit, todoCommands);

    // Resolve conflict
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");

    auto continueRebaseResult = rebase.continueRebase("New msg", "New desc");


    ASSERT_TRUE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.value(), commits.getHeadCommitHash());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getDescription(), "");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "New msg");
    EXPECT_EQ(commitsLog[1].getDescription(), "New desc");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}

TEST_F(RebaseInteractiveSquashFixupTest, interactive_conflictDuringSquashFixup_onFixup_breakAfterContinue)
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
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file1.txt");
    index.add("file4.txt");
    auto fifthCommit = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
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
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description\n\nFourth commit");
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
                          + "squash " + fourthCommit + " Fourth commit\n"
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

TEST_F(RebaseInteractiveSquashFixupTest, interactive_conflictDuringSquashFixup_onSquash_stop)
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
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description");
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
                          + "squash " + fourthCommit + " Fourth commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "rebase-merge" / "amend"), commitsLog[1].getHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "rebase-merge" / "current-fixup"), "squash " + fourthCommit + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "rebase-merge" / "rewritten-pending"), thirdCommit + "\n");
}

TEST_F(RebaseInteractiveSquashFixupTest, interactive_conflictDuringSquashFixup_onSquash_continue)
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
    auto fifthCommit = commits.createCommit("Fifth commit", "Fifth commit description");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
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
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description\n\nFourth commit\n\nFourth commit description");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}


TEST_F(RebaseInteractiveSquashFixupTest, interactive_conflictDuringSquashFixup_onSquash_breakAfterContinue)
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
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
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
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description\n\nFourth commit");
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
                          + "squash " + fourthCommit + " Fourth commit\n"
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
