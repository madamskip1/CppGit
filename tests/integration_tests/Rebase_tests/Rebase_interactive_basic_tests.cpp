
#include "Branches.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"
#include "RebaseFixture.hpp"
#include "RebaseTodoCommand.hpp"
#include "_details/FileUtility.hpp"

#include <gtest/gtest.h>

class RebaseInteractiveBasicTests : public RebaseFixture
{
};

TEST_F(RebaseInteractiveBasicTests, getTodoCommandsList_anotherOnto)
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

TEST_F(RebaseInteractiveBasicTests, getTodoCommandsList)
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


TEST_F(RebaseInteractiveBasicTests, ontoAnotherBranch)
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
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
}

TEST_F(RebaseInteractiveBasicTests, sameBranch)
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
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
}
