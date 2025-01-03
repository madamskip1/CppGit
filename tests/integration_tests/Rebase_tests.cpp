#include "BaseRepositoryFixture.hpp"
#include "Branches.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"
#include "RebaseTodoCommand.hpp"
#include "_details/FileUtility.hpp"
#include "_details/RebaseFilesHelper.hpp"

#include <gtest/gtest.h>

class RebaseTests : public BaseRepositoryFixture
{
};


TEST_F(RebaseTests, simpleRebase)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    auto secondCommit = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto fourthCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Fourth commit", { thirdCommitHash }, envp);

    auto rebaseResult = rebase.rebase("main");


    ASSERT_TRUE(rebaseResult.has_value());
    EXPECT_EQ(branches.getCurrentBranch(), "refs/heads/second_branch");
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 4);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[3].getMessage(), "Fourth commit");
    auto headCommitInfo = commits.getCommitInfo(commits.getHeadCommitHash());
    checkCommitAuthorEqualTest(headCommitInfo);
    checkCommitCommiterNotEqualTest(headCommitInfo);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));
    EXPECT_FALSE(std::filesystem::exists(repository->getGitDirectoryPath() / "rebase-merge"));
}

TEST_F(RebaseTests, simpleTodoCommandsList)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    auto secondCommit = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto fourthCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Fourth commit", { thirdCommitHash }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands("main");

    ASSERT_EQ(todoCommands.size(), 2);
    const auto& firstCommand = todoCommands[0];
    EXPECT_EQ(firstCommand.type, CppGit::RebaseTodoCommandType::PICK);
    EXPECT_EQ(firstCommand.hash, thirdCommitHash);
    EXPECT_EQ(firstCommand.message, "Third commit");
    const auto& secondCommand = todoCommands[1];
    EXPECT_EQ(secondCommand.type, CppGit::RebaseTodoCommandType::PICK);
    EXPECT_EQ(secondCommand.hash, fourthCommitHash);
    EXPECT_EQ(secondCommand.message, "Fourth commit");
}

TEST_F(RebaseTests, simpleTodoCommandsList_sameUpstream)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
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
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto fourthCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Fourth commit", { thirdCommitHash }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(secondCommit);

    ASSERT_EQ(todoCommands.size(), 2);
    const auto& firstCommand = todoCommands[0];
    EXPECT_EQ(firstCommand.type, CppGit::RebaseTodoCommandType::PICK);
    EXPECT_EQ(firstCommand.hash, thirdCommitHash);
    EXPECT_EQ(firstCommand.message, "Third commit");
    const auto& secondCommand = todoCommands[1];
    EXPECT_EQ(secondCommand.type, CppGit::RebaseTodoCommandType::PICK);
    EXPECT_EQ(secondCommand.hash, fourthCommitHash);
    EXPECT_EQ(secondCommand.message, "Fourth commit");
}


TEST_F(RebaseTests, rebaseConflict_firstCommit)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    index.add("file.txt");
    auto secondCommit = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second2");
    index.add("file.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");

    auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);
    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommit);
    auto todoFileExpected = "pick " + fourthCommit + " Fourth commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), todoFileExpected);
    auto doneFileExpected = "pick " + thirdCommitHash + " Third commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "stopped-sha"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nMain\n=======\nSecond\n>>>>>>> " + thirdCommitHash + "\n");


    // At that point we should only have commits from main branch (Initial commit and Second commit)
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
}

TEST_F(RebaseTests, rebaseConflict_notFirstCommit)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    index.add("file.txt");
    auto secondCommit = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    auto thirdCommitHash = commits.createCommit("Third commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");

    auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);
    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "pick " + thirdCommitHash + " Third commit\n"
                          + "pick " + fourthCommit + " Fourth commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "stopped-sha"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nMain\n=======\nSecond\n>>>>>>> " + fourthCommit + "\n");

    // At that point we should have commits from main branch and few from second branch
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
}

TEST_F(RebaseTests, rebaseConflict_bothConflictAndNotFiles)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    index.add("file1.txt");
    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 1!");
    index.add("file1.txt");
    auto secondCommit = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!");
    index.add("file1.txt");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);
    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "pick " + thirdCommitHash + " Third commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "stopped-sha"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\nHello, World! Modified 1!\n=======\nHello, World! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World 2!");

    // At that point we should only have commits from main branch (Initial commit and Second commit)
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
}

TEST_F(RebaseTests, rebaseConflict_conflictTwoFiles)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!");
    index.add("file1.txt");
    index.add("file2.txt");
    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 1!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2! Modified 1!");
    index.add("file1.txt");
    index.add("file2.txt");
    auto secondCommit = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2! Modified 2!");
    index.add("file1.txt");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);
    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "pick " + thirdCommitHash + " Third commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "stopped-sha"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\nHello, World! Modified 1!\n=======\nHello, World! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "<<<<<<< HEAD\nHello, World 2! Modified 1!\n=======\nHello, World 2! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");

    // At that point we should only have commits from main branch (Initial commit and Second commit)
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
}

TEST_F(RebaseTests, rebaseConflict_abort)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    index.add("file.txt");
    commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    rebase.rebase("main");

    auto rebaseAbortResult = rebase.abortRebase();


    ASSERT_TRUE(rebaseAbortResult == CppGit::Error::NO_ERROR);
    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir));
    auto currentBranch = branches.getCurrentBranch();
    EXPECT_EQ(currentBranch, "refs/heads/second_branch");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranch), thirdCommitHash);
    EXPECT_EQ(commits.getHeadCommitHash(), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Second");
}

TEST_F(RebaseTests, rebaseConflict_resolveContinue)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    index.add("file.txt");
    commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto thirdCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Third commit", { initialCommitHash }, envp);

    rebase.rebase("main");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Resolved");
    index.add("file.txt");

    auto rebaseContinueResult = rebase.continueRebase();


    ASSERT_TRUE(rebaseContinueResult.has_value());
    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir));
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Resolved");
    auto headCommitInfo = commits.getCommitInfo(commits.getHeadCommitHash());
    checkCommitAuthorEqualTest(headCommitInfo);
    checkCommitCommiterNotEqualTest(headCommitInfo);
}

TEST_F(RebaseTests, abort_noRebaseInProgress)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();


    commits.createCommit("Initial commit");
    auto rebaseAbortResult = rebase.abortRebase();


    EXPECT_EQ(rebaseAbortResult, CppGit::Error::NO_REBASE_IN_PROGRESS);
}

TEST_F(RebaseTests, continue_noRebaseInProgress)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();


    commits.createCommit("Initial commit");
    auto rebaseContinueResult = rebase.continueRebase();


    ASSERT_FALSE(rebaseContinueResult.has_value());
    EXPECT_EQ(rebaseContinueResult.error(), CppGit::Error::NO_REBASE_IN_PROGRESS);
}

TEST_F(RebaseTests, interactive_withAnotherUpstream)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    auto secondCommit = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto fourthCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Fourth commit", { thirdCommitHash }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands("main");

    auto rebaseResult = rebase.interactiveRebase("main", todoCommands);

    ASSERT_TRUE(rebaseResult.has_value());
    EXPECT_EQ(branches.getCurrentBranch(), "refs/heads/second_branch");
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 4);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[3].getMessage(), "Fourth commit");
    auto headCommitInfo = commits.getCommitInfo(commits.getHeadCommitHash());
    checkCommitAuthorEqualTest(headCommitInfo);
    checkCommitCommiterNotEqualTest(headCommitInfo);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));
    EXPECT_FALSE(std::filesystem::exists(repository->getGitDirectoryPath() / "rebase-merge"));
}

TEST_F(RebaseTests, interactive_sameUpstream)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
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
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto fourthCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Fourth commit", { thirdCommitHash }, envp);

    auto todoCommands = rebase.getDefaultTodoCommands(secondCommit);

    auto rebaseResult = rebase.interactiveRebase(secondCommit, todoCommands);


    ASSERT_TRUE(rebaseResult.has_value());
    EXPECT_EQ(branches.getCurrentBranch(), "refs/heads/main");
    // We did just FasForward for all commits
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 4);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[2].getHash(), thirdCommitHash);
    EXPECT_EQ(commitsLog[3].getMessage(), "Fourth commit");
    EXPECT_EQ(commitsLog[3].getHash(), fourthCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));
    EXPECT_FALSE(std::filesystem::exists(repository->getGitDirectoryPath() / "rebase-merge"));
}

TEST_F(RebaseTests, interactive_break)
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
}

TEST_F(RebaseTests, interactive_break_continue)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
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
    todoCommands.insert(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);

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

TEST_F(RebaseTests, interactive_reword_stop_fastForward)
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
    // At that point we should applied second commit fast forward
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "COMMIT_EDITMSG"), "Second commit");
    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_TRUE(std::filesystem::exists(gitRebaseDir));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "reword " + secondCommit + " Second commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    auto authorScriptFileContent = CppGit::_details::RebaseFilesHelper{ *repository }.getAuthorScriptFile();
    EXPECT_EQ(authorScriptFileContent[0], envp[0]); // Author Name
    EXPECT_EQ(authorScriptFileContent[1], envp[1]); // Author email
    EXPECT_EQ(authorScriptFileContent[2], envp[2]); // Author date
}

TEST_F(RebaseTests, interactive_reword_stop_noFastForward)
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
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "COMMIT_EDITMSG"), "Third commit");
    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_TRUE(std::filesystem::exists(gitRebaseDir));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "drop " + secondCommit + " Second commit\n"
                          + "reword " + thirdCommit + " Third commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    auto authorScriptFileContent = CppGit::_details::RebaseFilesHelper{ *repository }.getAuthorScriptFile();
    EXPECT_EQ(authorScriptFileContent[0], envp[0]); // Author Name
    EXPECT_EQ(authorScriptFileContent[1], envp[1]); // Author email
    EXPECT_EQ(authorScriptFileContent[2], envp[2]); // Author date
}

TEST_F(RebaseTests, interactive_reword_continue_fastForward_changeMessage)
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

    auto continueRewordResult = rebase.continueReword("New message");


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
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}

TEST_F(RebaseTests, interactive_reword_continue_fastForward_noChangeMessage)
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

    auto continueRewordResult = rebase.continueReword();


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
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}

TEST_F(RebaseTests, interactive_reword_continue_noFastForward_changeMessage)
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

    auto continueRewordResult = rebase.continueReword("New message");


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
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}

TEST_F(RebaseTests, interactive_reword_continue_noFastForward_noChangeMessage)
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

    auto continueRewordResult = rebase.continueReword();


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
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}

TEST_F(RebaseTests, interactive_reword_continue_withDescription)
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

    auto continueRewordResult = rebase.continueReword("New message", "New description");


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
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}

TEST_F(RebaseTests, interactive_reword_continue_oldMessage)
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

    auto continueRewordResult = rebase.continueReword();


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
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}

TEST_F(RebaseTests, interactive_edit_stop)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);

    auto initialCommit = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello World!");
    index.add("file1.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommit }, envp);
    auto thirdCommit = commits.createCommit("Third commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::EDIT;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_EDIT);
    EXPECT_EQ(commits.getHeadCommitHash(), secondCommit);
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    checkCommitAuthorEqualTest(commits.getCommitInfo(commitsLog[1].getHash()));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_TRUE(std::filesystem::exists(gitRebaseDir));
    auto todoFileExpected = "pick " + thirdCommit + " Third commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), todoFileExpected);
    auto doneFileExpected = "edit " + secondCommit + " Second commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "amend"), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "stopped-sha"), secondCommit);
    auto authorScriptFileContent = CppGit::_details::RebaseFilesHelper{ *repository }.getAuthorScriptFile();
    EXPECT_EQ(authorScriptFileContent[0], envp[0]); // Author Name
    EXPECT_EQ(authorScriptFileContent[1], envp[1]); // Author email
    EXPECT_EQ(authorScriptFileContent[2], envp[2]); // Author date
}

TEST_F(RebaseTests, interactive_edit_continue_changes)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);

    auto initialCommit = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World!");
    index.add("file1.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommit = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommit }, envp);
    auto thirdCommit = commits.createCommit("Third commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommit);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::EDIT;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_EDIT);

    CppGit::_details::FileUtility::createOrAppendFile(repositoryPath / "file1.txt", " Modified");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file1.txt");
    index.add("file2.txt");

    auto continueEditResult = rebase.continueRebase();

    EXPECT_NE(commits.getHeadCommitHash(), thirdCommit); // we couldn't do FastForward
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_NE(commitsLog[1].getHash(), secondCommit);
    checkCommitAuthorEqualTest(commits.getCommitInfo(commitsLog[1].getHash()));
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_NE(commitsLog[2].getHash(), thirdCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World! Modified");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "rebase-merge"));
}

TEST_F(RebaseTests, interactive_edit_continue_noChanges)
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
    todoCommands[0].type = CppGit::RebaseTodoCommandType::EDIT;

    auto rebaseResult = rebase.interactiveRebase(initialCommit, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_EDIT);

    auto continueEditResult = rebase.continueRebase();


    EXPECT_EQ(commits.getHeadCommitHash(), thirdCommit); // we could do FastForward
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    checkCommitAuthorEqualTest(commits.getCommitInfo(commitsLog[1].getHash()));
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[2].getHash(), thirdCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello World!");
}

TEST_F(RebaseTests, interactive_drop)
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
