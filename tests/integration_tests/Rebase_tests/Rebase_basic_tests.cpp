
#include "CppGit/Branches.hpp"
#include "CppGit/Commits.hpp"
#include "CppGit/CommitsHistory.hpp"
#include "CppGit/Index.hpp"
#include "CppGit/Rebase.hpp"
#include "CppGit/_details/FileUtility.hpp"
#include "RebaseFixture.hpp"

#include <gtest/gtest.h>

class RebaseBasicTests : public RebaseFixture
{
};

TEST_F(RebaseBasicTests, simpleRebase)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    const auto rebaseResult = rebase.rebase("main");


    ASSERT_TRUE(rebaseResult.has_value());

    EXPECT_EQ(commits.getHeadCommitHash(), rebaseResult.value());
    const auto currentBranchName = branches.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/second_branch");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranchName), rebaseResult.value());

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
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
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseBasicTests, conflictOnFirstCommit_stop)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second2");
    index.add("file.txt");
    const auto fourthCommitHash = commits.createCommit("Fourth commit");

    const auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nMain\n=======\nSecond\n>>>>>>> " + thirdCommitHash + "\n");

    constexpr auto* expectedMessage = "Third commit\n\nThird commit description";
    EXPECT_EQ(rebase.getStoppedMessage(), expectedMessage);

    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), "pick " + thirdCommitHash + " Third commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "pick " + fourthCommitHash + " Fourth commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseBasicTests, conflictOnNotFirstCommit_stop)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    const auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nMain\n=======\nSecond\n>>>>>>> " + fourthCommitHash + "\n");

    constexpr auto* expectedMessage = "Fourth commit";
    EXPECT_EQ(rebase.getStoppedMessage(), expectedMessage);

    const auto doneFileExpected = "pick " + thirdCommitHash + " Third commit\n"
                                + "pick " + fourthCommitHash + " Fourth commit\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), thirdCommitHash + " " + commitsLog[2].getHash() + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseBasicTests, conflict_bothConflictedAndNotFiles_stop)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    index.add("file1.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 1!");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!");
    index.add("file1.txt");
    index.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);

    const auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\nHello, World! Modified 1!\n=======\nHello, World! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World 2!");

    constexpr auto* expectedMessage = "Third commit";
    EXPECT_EQ(rebase.getStoppedMessage(), expectedMessage);

    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
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
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseBasicTests, conflict_conflictedTwoFiles_stop)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!");
    index.add("file1.txt");
    index.add("file2.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 1!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2! Modified 1!");
    index.add("file1.txt");
    index.add("file2.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2! Modified 2!");
    index.add("file1.txt");
    index.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);

    const auto rebaseResult = rebase.rebase("main");


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\nHello, World! Modified 1!\n=======\nHello, World! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "<<<<<<< HEAD\nHello, World 2! Modified 1!\n=======\nHello, World 2! Modified 2!\n>>>>>>> " + thirdCommitHash + "\n");

    constexpr auto* expectedMessage = "Third commit";
    EXPECT_EQ(rebase.getStoppedMessage(), expectedMessage);

    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "author-script"), expectedAuthorScript);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), "pick " + thirdCommitHash + " Third commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseBasicTests, conflict_abort)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();


    createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    index.add("file.txt");
    commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");

    const auto rebaseResult = rebase.rebase("main");
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    const auto rebaseAbortResult = rebase.abortRebase();


    ASSERT_EQ(rebaseAbortResult, CppGit::Error::NO_ERROR);

    EXPECT_EQ(commits.getHeadCommitHash(), thirdCommitHash);
    const auto currentBranchName = branches.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/second_branch");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranchName), thirdCommitHash);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Second");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseBasicTests, conflict_continue)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    index.add("file.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", initialCommitHash);

    const auto rebaseResult = rebase.rebase("main");
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Resolved");
    index.add("file.txt");
    const auto rebaseContinueResult = rebase.continueRebase();


    ASSERT_TRUE(rebaseContinueResult.has_value());

    EXPECT_EQ(commits.getHeadCommitHash(), rebaseContinueResult.value());
    const auto currentBranchName = branches.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/second_branch");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranchName), rebaseContinueResult.value());

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[2].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(commitsLog[2]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Resolved");


    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseBasicTests, abort_noRebaseInProgress)
{
    const auto rebase = repository->Rebase();

    createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    const auto rebaseAbortResult = rebase.abortRebase();

    EXPECT_EQ(rebaseAbortResult, CppGit::Error::NO_REBASE_IN_PROGRESS);
}

TEST_F(RebaseBasicTests, continue_noRebaseInProgress)
{
    const auto rebase = repository->Rebase();

    createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    const auto rebaseContinueResult = rebase.continueRebase();

    ASSERT_FALSE(rebaseContinueResult.has_value());
    EXPECT_EQ(rebaseContinueResult.error(), CppGit::Error::NO_REBASE_IN_PROGRESS);
}
