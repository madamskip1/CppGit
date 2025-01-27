#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"
#include "RebaseFixture.hpp"
#include "RebaseTodoCommand.hpp"
#include "_details/FileUtility.hpp"
#include "_details/RebaseFilesHelper.hpp"

#include <gtest/gtest.h>

class RebaseInteractiveRewordTests : public RebaseFixture
{
};


TEST_F(RebaseInteractiveRewordTests, fastForward_stop)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    index.add("file.txt");
    auto secondCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", "Second commit description", { initialCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::REWORD;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_REWORD);
    // At that point we should applied second commit fast forward
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");

    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "stopped-sha"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commits.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), "reword " + secondCommit + " Second commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), "Second commit\n\nSecond commit description");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), secondCommit);
}

TEST_F(RebaseInteractiveRewordTests, noFastForward_stop)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    auto secondCommit = commits.createCommit("Second commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    index.add("file.txt");
    auto thirdCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Third commit", { secondCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[1].type = CppGit::RebaseTodoCommandType::REWORD;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_REWORD);
    // At that point we should applied second commit fast forward
    EXPECT_NE(commits.getHeadCommitHash(), thirdCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");

    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "reword " + thirdCommit + " Third commit\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "stopped-sha"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), thirdCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commits.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), "Third commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), thirdCommit);
}

TEST_F(RebaseInteractiveRewordTests, fastForward_continue_changeMessage)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    index.add("file.txt");
    auto secondCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::REWORD;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_REWORD);

    auto continueRewordResult = rebase.continueRebase("New message");


    ASSERT_TRUE(continueRewordResult.has_value());
    EXPECT_NE(continueRewordResult.value(), secondCommit);
    EXPECT_EQ(commits.getHeadCommitHash(), continueRewordResult.value());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "New message");
    EXPECT_EQ(commitsLog[1].getDescription(), "");
    EXPECT_EQ(commitsLog[1].getHash(), continueRewordResult.value());
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");
    checkCommitAuthorEqualTest(commits.getCommitInfo(continueRewordResult.value()));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}

TEST_F(RebaseInteractiveRewordTests, fastForward_continue_noChangeMessage)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);

    auto initialCommit = commits.createCommit("Initial commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    index.add("file.txt");
    auto secondCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::REWORD;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_REWORD);

    auto continueRewordResult = rebase.continueRebase();


    ASSERT_TRUE(continueRewordResult.has_value());
    EXPECT_NE(continueRewordResult.value(), secondCommit);
    EXPECT_EQ(commits.getHeadCommitHash(), continueRewordResult.value());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "");
    EXPECT_EQ(commitsLog[1].getHash(), continueRewordResult.value());
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");
    checkCommitAuthorEqualTest(commits.getCommitInfo(continueRewordResult.value()));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}

TEST_F(RebaseInteractiveRewordTests, noFastForward_continue_changeMesage)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    auto secondCommit = commits.createCommit("Second commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    index.add("file.txt");
    auto thirdCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Third commit", { secondCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[1].type = CppGit::RebaseTodoCommandType::REWORD;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_REWORD);

    auto continueRewordResult = rebase.continueRebase("New message");


    ASSERT_TRUE(continueRewordResult.has_value());
    EXPECT_NE(continueRewordResult.value(), thirdCommit);
    EXPECT_EQ(commits.getHeadCommitHash(), continueRewordResult.value());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "New message");
    EXPECT_EQ(commitsLog[1].getDescription(), "");
    EXPECT_EQ(commitsLog[1].getHash(), continueRewordResult.value());
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");
    checkCommitAuthorEqualTest(commits.getCommitInfo(continueRewordResult.value()));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}

TEST_F(RebaseInteractiveRewordTests, noFastForward_continue_NoChangeMessage)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    auto secondCommit = commits.createCommit("Second commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    index.add("file.txt");
    auto thirdCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Third commit", { secondCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[1].type = CppGit::RebaseTodoCommandType::REWORD;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_REWORD);

    auto continueRewordResult = rebase.continueRebase();


    ASSERT_TRUE(continueRewordResult.has_value());
    EXPECT_NE(continueRewordResult.value(), thirdCommit);
    EXPECT_EQ(commits.getHeadCommitHash(), continueRewordResult.value());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "");
    EXPECT_EQ(commitsLog[1].getHash(), continueRewordResult.value());
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");
    checkCommitAuthorEqualTest(commits.getCommitInfo(continueRewordResult.value()));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}

TEST_F(RebaseInteractiveRewordTests, continue_changeMessageAndDescription)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    index.add("file.txt");
    auto secondCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::REWORD;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_REWORD);

    auto continueRewordResult = rebase.continueRebase("New message", "New description");


    ASSERT_TRUE(continueRewordResult.has_value());
    EXPECT_NE(continueRewordResult.value(), secondCommit); // we create a new commit
    EXPECT_EQ(commits.getHeadCommitHash(), continueRewordResult.value());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "New message");
    EXPECT_EQ(commitsLog[1].getDescription(), "New description");
    EXPECT_EQ(commitsLog[1].getHash(), continueRewordResult.value());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");
    checkCommitAuthorEqualTest(commits.getCommitInfo(continueRewordResult.value()));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}

TEST_F(RebaseInteractiveRewordTests, continue_keepOldMessage)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    index.add("file.txt");
    auto secondCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommit }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::REWORD;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_REWORD);

    auto continueRewordResult = rebase.continueRebase();


    ASSERT_TRUE(continueRewordResult.has_value());
    EXPECT_NE(continueRewordResult.value(), secondCommit); // we create a new commit
    EXPECT_EQ(commits.getHeadCommitHash(), continueRewordResult.value());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), continueRewordResult.value());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");
    checkCommitAuthorEqualTest(commits.getCommitInfo(continueRewordResult.value()));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}

TEST_F(RebaseInteractiveRewordTests, breakAfter)
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
    todoCommands[0].type = CppGit::RebaseTodoCommandType::REWORD;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);
    rebase.interactiveRebase(initialCommit, todoCommands);

    auto continueRewordResult = rebase.continueRebase("New message");


    ASSERT_FALSE(continueRewordResult.has_value());
    EXPECT_EQ(continueRewordResult.error(), CppGit::Error::REBASE_BREAK);
    EXPECT_NE(commits.getHeadCommitHash(), secondCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "New message");
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    auto secondCommitInfo = commits.getCommitInfo(commitsLog[1].getHash());
    checkCommitAuthorEqualTest(secondCommitInfo);
    checkCommitCommiterNotEqualTest(secondCommitInfo);

    auto doneFileExpected = "reword " + secondCommit + " Second commit\n"
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
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), secondCommit + " " + commits.getHeadCommitHash() + "\n");
}

TEST_F(RebaseInteractiveRewordTests, conflict_stop)
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
    todoCommands[2].type = CppGit::RebaseTodoCommandType::REWORD;

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

    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "pick " + thirdCommit + " Third commit\n"
                          + "reword " + fourthCommit + " Fourth commit\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), "Fourth commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), thirdCommit + " " + commitsLog[1].getHash() + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "stopped-sha"), fourthCommit);
}

TEST_F(RebaseInteractiveRewordTests, conflict_continue)
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
    todoCommands[2].type = CppGit::RebaseTodoCommandType::REWORD;

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
    EXPECT_EQ(commitsLog[0].getDescription(), "");
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

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}

TEST_F(RebaseInteractiveRewordTests, conflict_continue_changeMesssage)
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
    todoCommands[2].type = CppGit::RebaseTodoCommandType::REWORD;

    rebase.interactiveRebase(initialCommit, todoCommands);

    // Resolve conflict
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");

    auto continueRebaseResult = rebase.continueRebase("New msg");


    ASSERT_TRUE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.value(), commits.getHeadCommitHash());
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_NE(commitsLog[1].getHash(), thirdCommit);
    EXPECT_EQ(commitsLog[2].getMessage(), "New msg");
    EXPECT_NE(commitsLog[2].getHash(), fourthCommit);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}

TEST_F(RebaseInteractiveRewordTests, conflict_breakAfter)
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
    todoCommands[2].type = CppGit::RebaseTodoCommandType::REWORD;
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

    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "pick " + thirdCommit + " Third commit\n"
                          + "reword " + fourthCommit + " Fourth commit\n"
                          + "break\n";
    auto rewrittenListExpected = thirdCommit + " " + commitsLog[1].getHash() + "\n"
                               + fourthCommit + " " + commitsLog[2].getHash() + "\n";
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
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
}
