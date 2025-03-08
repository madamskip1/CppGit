
#include "CppGit/Branches.hpp"
#include "CppGit/Commits.hpp"
#include "CppGit/CommitsHistory.hpp"
#include "CppGit/Index.hpp"
#include "CppGit/Rebase.hpp"
#include "CppGit/RebaseTodoCommand.hpp"
#include "CppGit/_details/FileUtility.hpp"
#include "RebaseFixture.hpp"

#include <gtest/gtest.h>

class RebaseInteractiveBasicTests : public RebaseFixture
{
};

TEST_F(RebaseInteractiveBasicTests, getTodoCommandsList_anotherOnto)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    const auto todoCommands = rebase.getDefaultTodoCommands("main");

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

TEST_F(RebaseInteractiveBasicTests, getTodoCommandsList)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    const auto todoCommands = rebase.getDefaultTodoCommands(secondCommitHash);

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


TEST_F(RebaseInteractiveBasicTests, ontoAnotherBranch)
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

    const auto todoCommands = rebase.getDefaultTodoCommands("main");

    const auto rebaseResult = rebase.interactiveRebase("main", todoCommands);

    ASSERT_TRUE(rebaseResult.has_value());

    EXPECT_EQ(commits.getHeadCommitHash(), rebaseResult.value());
    const auto currentBranchName = branches.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/second_branch");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranchName), rebaseResult.value());
    EXPECT_EQ(branches.getCurrentBranchName(), "refs/heads/second_branch");

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

TEST_F(RebaseInteractiveBasicTests, sameBranch)
{
    const auto commits = repository->Commits();
    const auto branches = repository->Branches();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");
    const auto fourthCommitHash = commits.createCommit("Fourth commit", thirdCommitHash);

    const auto todoCommands = rebase.getDefaultTodoCommands(secondCommitHash);

    const auto rebaseResult = rebase.interactiveRebase(secondCommitHash, todoCommands);


    ASSERT_TRUE(rebaseResult.has_value());

    EXPECT_EQ(commits.getHeadCommitHash(), rebaseResult.value());
    const auto currentBranchName = branches.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranchName), rebaseResult.value());

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 4);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getHash(), secondCommitHash);
    EXPECT_EQ(commitsLog[2].getHash(), thirdCommitHash);
    EXPECT_EQ(commitsLog[3].getHash(), fourthCommitHash);

    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}
