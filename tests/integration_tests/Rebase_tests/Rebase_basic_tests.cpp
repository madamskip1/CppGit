
#include "../BaseRepositoryFixture.hpp"
#include "Branches.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"
#include "_details/FileUtility.hpp"

#include <gtest/gtest.h>

class RebaseBasicTests : public BaseRepositoryFixture
{
};

TEST_F(RebaseBasicTests, simpleRebase)
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

TEST_F(RebaseBasicTests, conflictOnFirstCommit_stop)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommithash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

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
    // At that point we should only have commits from main branch (Initial commit and Second commit)
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommithash);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nMain\n=======\nSecond\n>>>>>>> " + thirdCommitHash + "\n");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), fourthCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommit);
    auto todoFileExpected = "pick " + fourthCommit + " Fourth commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), todoFileExpected);
    auto doneFileExpected = "pick " + thirdCommitHash + " Third commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "stopped-sha"), thirdCommitHash);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "rewritten-list"));
}

TEST_F(RebaseBasicTests, conflictOnNotFirstCommit_stop)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
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
    // At that point we should have commits from main branch and few from second branch
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_NE(commitsLog[2].getHash(), thirdCommitHash);
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nMain\n=======\nSecond\n>>>>>>> " + fourthCommit + "\n");

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
    auto rewrittenListFileExpected = thirdCommitHash + " " + commitsLog[2].getHash() + "\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "rewritten-list"), rewrittenListFileExpected);
}

TEST_F(RebaseBasicTests, conflict_bothConflictedAndNotFiles_stop)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    index.add("file1.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 1!");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!");
    index.add("file1.txt");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);
    // At that point we should only have commits from main branch (Initial commit and Second commit)
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\nHello, World! Modified 1!\n=======\nHello, World! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World 2!");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "pick " + thirdCommitHash + " Third commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "stopped-sha"), thirdCommitHash);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "rewritten-list"));
}

TEST_F(RebaseBasicTests, conflict_conflictedTwoFiles_stop)
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
    auto initialCommitHash = commits.createCommit("Initial commit");
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
    // At that point we should only have commits from main branch (Initial commit and Second commit)
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\nHello, World! Modified 1!\n=======\nHello, World! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "<<<<<<< HEAD\nHello, World 2! Modified 1!\n=======\nHello, World 2! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "onto"), secondCommit);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "git-rebase-todo"), "");
    auto doneFileExpected = "pick " + thirdCommitHash + " Third commit\n";
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitRebaseDir / "stopped-sha"), thirdCommitHash);
    EXPECT_FALSE(std::filesystem::exists(gitRebaseDir / "rewritten-list"));
}

TEST_F(RebaseBasicTests, conflict_abort)
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

TEST_F(RebaseBasicTests, conflict_continue)
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
    auto thirdCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Third commit", "Third commit description", { initialCommitHash }, envp);

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
    EXPECT_EQ(commitsLog[2].getDescription(), "Third commit description");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Resolved");
    auto headCommitInfo = commits.getCommitInfo(commits.getHeadCommitHash());
    checkCommitAuthorEqualTest(headCommitInfo);
    checkCommitCommiterNotEqualTest(headCommitInfo);
}

TEST_F(RebaseBasicTests, abort_noRebaseInProgress)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();


    commits.createCommit("Initial commit");
    auto rebaseAbortResult = rebase.abortRebase();


    EXPECT_EQ(rebaseAbortResult, CppGit::Error::NO_REBASE_IN_PROGRESS);
}

TEST_F(RebaseBasicTests, continue_noRebaseInProgress)
{
    auto commits = repository->Commits();
    auto rebase = repository->Rebase();


    commits.createCommit("Initial commit");
    auto rebaseContinueResult = rebase.continueRebase();


    ASSERT_FALSE(rebaseContinueResult.has_value());
    EXPECT_EQ(rebaseContinueResult.error(), CppGit::Error::NO_REBASE_IN_PROGRESS);
}
