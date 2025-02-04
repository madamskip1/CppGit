
#include "Branches.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"
#include "RebaseFixture.hpp"
#include "_details/FileUtility.hpp"

#include <gtest/gtest.h>

class RebaseBasicTests : public RebaseFixture
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


    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);
    createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    auto rebaseResult = rebase.rebase("main");


    ASSERT_TRUE(rebaseResult.has_value());

    EXPECT_EQ(commits.getHeadCommitHash(), rebaseResult.value());
    auto currentBranch = branches.getCurrentBranch();
    EXPECT_EQ(currentBranch, "refs/heads/second_branch");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranch), rebaseResult.value());

    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 4);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[2]);
    EXPECT_EQ(commitsLog[3].getMessage(), "Fourth commit");
    EXPECT_EQ(commitsLog[3].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[3]);

    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}

TEST_F(RebaseBasicTests, conflictOnFirstCommit_stop)
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
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second2");
    index.add("file.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit");

    auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nMain\n=======\nSecond\n>>>>>>> " + thirdCommitHash + "\n");

    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), "pick " + thirdCommitHash + " Third commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "pick " + fourthCommitHash + " Fourth commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), "Third commit\n\nThird commit description");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "stopped-sha"), thirdCommitHash);
}

TEST_F(RebaseBasicTests, conflictOnNotFirstCommit_stop)
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
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nMain\n=======\nSecond\n>>>>>>> " + fourthCommitHash + "\n");

    auto doneFileExpected = "pick " + thirdCommitHash + " Third commit\n"
                          + "pick " + fourthCommitHash + " Fourth commit\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), "Fourth commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), thirdCommitHash + " " + commitsLog[2].getHash() + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "stopped-sha"), fourthCommitHash);
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
    auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);

    auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\nHello, World! Modified 1!\n=======\nHello, World! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World 2!");

    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), "pick " + thirdCommitHash + " Third commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), "Third commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "stopped-sha"), thirdCommitHash);
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
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2! Modified 2!");
    index.add("file1.txt");
    index.add("file2.txt");
    auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);

    auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\nHello, World! Modified 1!\n=======\nHello, World! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "<<<<<<< HEAD\nHello, World 2! Modified 1!\n=======\nHello, World 2! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");

    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixup"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), "pick " + thirdCommitHash + " Third commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), "Third commit");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "stopped-sha"), thirdCommitHash);
}

TEST_F(RebaseBasicTests, conflict_abort)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();


    createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    index.add("file.txt");
    commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto rebaseResult = rebase.rebase("main");
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    auto rebaseAbortResult = rebase.abortRebase();


    ASSERT_EQ(rebaseAbortResult, CppGit::Error::NO_ERROR);

    EXPECT_EQ(commits.getHeadCommitHash(), thirdCommitHash);
    auto currentBranch = branches.getCurrentBranch();
    EXPECT_EQ(currentBranch, "refs/heads/second_branch");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranch), thirdCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Second");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
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
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    createCommitWithTestAuthorCommiter("Third commit", "Third commit description", initialCommitHash);

    auto rebaseResult = rebase.rebase("main");
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Resolved");
    index.add("file.txt");
    auto rebaseContinueResult = rebase.continueRebase();


    ASSERT_TRUE(rebaseContinueResult.has_value());

    EXPECT_EQ(commits.getHeadCommitHash(), rebaseContinueResult.value());
    auto currentBranch = branches.getCurrentBranch();
    EXPECT_EQ(currentBranch, "refs/heads/second_branch");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranch), rebaseContinueResult.value());

    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[2].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(commitsLog[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Resolved");


    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
}

TEST_F(RebaseBasicTests, abort_noRebaseInProgress)
{
    auto rebase = repository->Rebase();

    createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    auto rebaseAbortResult = rebase.abortRebase();

    EXPECT_EQ(rebaseAbortResult, CppGit::Error::NO_REBASE_IN_PROGRESS);
}

TEST_F(RebaseBasicTests, continue_noRebaseInProgress)
{
    auto rebase = repository->Rebase();

    createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    auto rebaseContinueResult = rebase.continueRebase();

    ASSERT_FALSE(rebaseContinueResult.has_value());
    EXPECT_EQ(rebaseContinueResult.error(), CppGit::Error::NO_REBASE_IN_PROGRESS);
}
