#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"
#include "RebaseFixture.hpp"
#include "RebaseTodoCommand.hpp"
#include "_details/FileUtility.hpp"

#include <gtest/gtest.h>

class RebaseInteractiveSquashFixupTest : public RebaseFixture
{
};

TEST_F(RebaseInteractiveSquashFixupTest, stop)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;

    auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_SQUASH);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_NE(commitsLog[1].getHash(), thirdCommitHash);
    EXPECT_NE(commitsLog[1].getHash(), fourthCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commits.getHeadCommitHash(), commitsLog[1].getHash());
    auto messageExpected = "Second commit\n\nThird commit\n\nThird commit description";
    EXPECT_EQ(rebase.getStoppedMessage(), messageExpected);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                          + "squash " + thirdCommitHash + " Third commit\n"
                          + "fixup " + fourthCommitHash + " Fourth commit\n";
    auto rewrittenPendingExpected = secondCommitHash + "\n"
                                  + thirdCommitHash + "\n";
    auto currentFixupExpected = "squash " + thirdCommitHash + "\n"
                              + "fixup " + fourthCommitHash + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "stopped-sha"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commits.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "current-fixup"), currentFixupExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), "Second commit\n\nThird commit\n\nThird commit description");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-pending"), rewrittenPendingExpected);
}

TEST_F(RebaseInteractiveSquashFixupTest, continue_noChangeMessage)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;

    rebase.interactiveRebase(initialCommitHash, todoCommands);

    auto squashContinueResult = rebase.continueRebase();


    ASSERT_TRUE(squashContinueResult.has_value());
    EXPECT_EQ(squashContinueResult.value(), commits.getHeadCommitHash());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_NE(commitsLog[1].getHash(), thirdCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit\n\nThird commit description");
    EXPECT_EQ(commits.getHeadCommitHash(), commitsLog[1].getHash());

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}

TEST_F(RebaseInteractiveSquashFixupTest, breakAfter)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    rebase.interactiveRebase(initialCommitHash, todoCommands);

    auto squashContinueResult = rebase.continueRebase();


    ASSERT_FALSE(squashContinueResult.has_value());
    EXPECT_EQ(squashContinueResult.error(), CppGit::Error::REBASE_BREAK);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_NE(commitsLog[1].getHash(), thirdCommitHash);
    EXPECT_NE(commitsLog[1].getHash(), fourthCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit\n\nThird commit description");

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                          + "squash " + thirdCommitHash + " Third commit\n"
                          + "fixup " + fourthCommitHash + " Fourth commit\n"
                          + "break\n";
    auto rewrittenListExpected = secondCommitHash + " " + commitsLog[1].getHash() + "\n"
                               + thirdCommitHash + " " + commitsLog[1].getHash() + "\n"
                               + fourthCommitHash + " " + commitsLog[1].getHash() + "\n";
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
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
}

TEST_F(RebaseInteractiveSquashFixupTest, squashFixupAfterBreak_breakAfter)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    rebase.interactiveRebase(initialCommitHash, todoCommands);
    rebase.continueRebase(); // we was at first break

    auto squashContinueResult = rebase.continueRebase();


    ASSERT_FALSE(squashContinueResult.has_value());
    EXPECT_EQ(squashContinueResult.error(), CppGit::Error::REBASE_BREAK);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_NE(commitsLog[1].getHash(), thirdCommitHash);
    EXPECT_NE(commitsLog[1].getHash(), fourthCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit\n\nThird commit description");

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                          + "break\n"
                          + "squash " + thirdCommitHash + " Third commit\n"
                          + "fixup " + fourthCommitHash + " Fourth commit\n"
                          + "break\n";
    auto rewrittenListExpected = thirdCommitHash + " " + commitsLog[1].getHash() + "\n"
                               + fourthCommitHash + " " + commitsLog[1].getHash() + "\n";
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
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
}

TEST_F(RebaseInteractiveSquashFixupTest, conflictOnFixup_stop)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto thirdCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Third commit", "Third commit description", { secondCommitHash }, envp);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file1.txt");
    index.add("file4.txt");

    auto fifthCommitHash = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;

    auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description\n\nFourth commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\n=======\nHello World 1, new!\n>>>>>>> " + fifthCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                          + "pick " + thirdCommitHash + " Third commit\n"
                          + "squash " + fourthCommitHash + " Fourth commit\n"
                          + "fixup " + fifthCommitHash + " Fifth commit\n";
    auto currentFixupExpected = "squash " + fourthCommitHash + "\n"
                              + "fixup " + fifthCommitHash + "\n";
    auto rewrittenPendingExpected = thirdCommitHash + "\n"
                                  + fourthCommitHash + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commits.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript); // Czy tu nie powinno nie byc tego pliku?
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "current-fixup"), currentFixupExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), "Third commit\n\nThird commit description\n\nFourth commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-pending"), rewrittenPendingExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "stopped-sha"), fifthCommitHash);
}

TEST_F(RebaseInteractiveSquashFixupTest, conflictOnFixup_continue)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit", "Fourth commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file1.txt");
    index.add("file4.txt");
    auto fifthCommitHash = commits.createCommit("Fifth commit", "Fifth commit description");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;

    rebase.interactiveRebase(initialCommitHash, todoCommands);

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
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "New msg");
    EXPECT_EQ(commitsLog[1].getDescription(), "New desc");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}

TEST_F(RebaseInteractiveSquashFixupTest, conflictOnFixup_breakAfter)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file1.txt");
    index.add("file4.txt");
    auto fifthCommitHash = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    rebase.interactiveRebase(initialCommitHash, todoCommands); // now we are on conflict
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");

    auto continueRebaseResult = rebase.continueRebase(); // now we are on break


    ASSERT_FALSE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.error(), CppGit::Error::REBASE_BREAK);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description\n\nFourth commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                          + "pick " + thirdCommitHash + " Third commit\n"
                          + "squash " + fourthCommitHash + " Fourth commit\n"
                          + "fixup " + fifthCommitHash + " Fifth commit\n"
                          + "break\n";
    auto rewrittenListExpected = thirdCommitHash + " " + commitsLog[1].getHash() + "\n"
                               + fourthCommitHash + " " + commitsLog[1].getHash() + "\n"
                               + fifthCommitHash + " " + commitsLog[1].getHash() + "\n";
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
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
}

TEST_F(RebaseInteractiveSquashFixupTest, conflictOnSquash_stop)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file1.txt");
    index.add("file3.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    auto fifthCommitHash = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;

    auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\n=======\nHello World 1, new!\n>>>>>>> " + fourthCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file4.txt"));

    auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                          + "pick " + thirdCommitHash + " Third commit\n"
                          + "squash " + fourthCommitHash + " Fourth commit\n";
    ;
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commits.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "current-fixup"), "squash " + fourthCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "fixup " + fifthCommitHash + " Fifth commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), "Third commit\n\nThird commit description\n\nFourth commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-pending"), thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "stopped-sha"), fourthCommitHash);
}

TEST_F(RebaseInteractiveSquashFixupTest, conflictOnSquash_continue)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file1.txt");
    index.add("file3.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit", "Fourth commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    auto fifthCommitHash = commits.createCommit("Fifth commit", "Fifth commit description");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;

    rebase.interactiveRebase(initialCommitHash, todoCommands);

    // Resolve conflict
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");

    auto continueRebaseResult = rebase.continueRebase();


    ASSERT_TRUE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.value(), commits.getHeadCommitHash());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description\n\nFourth commit\n\nFourth commit description");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}


TEST_F(RebaseInteractiveSquashFixupTest, conflictOnSquash_breakAfter)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit", "Third commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file1.txt");
    index.add("file3.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    auto fifthCommitHash = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::SQUASH;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    rebase.interactiveRebase(initialCommitHash, todoCommands); // now we are on conflict
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");

    auto continueRebaseResult = rebase.continueRebase(); // now we are on break


    ASSERT_FALSE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.error(), CppGit::Error::REBASE_BREAK);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description\n\nFourth commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                          + "pick " + thirdCommitHash + " Third commit\n"
                          + "squash " + fourthCommitHash + " Fourth commit\n"
                          + "fixup " + fifthCommitHash + " Fifth commit\n"
                          + "break\n";
    auto rewrittenListExpected = thirdCommitHash + " " + commitsLog[1].getHash() + "\n"
                               + fourthCommitHash + " " + commitsLog[1].getHash() + "\n"
                               + fifthCommitHash + " " + commitsLog[1].getHash() + "\n";
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
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
}
